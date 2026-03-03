/*
 * This file is part of INAV.
 *
 * Adapted from Betaflight's dshot_bitbang implementation.
 * Original work Copyright (c) Betaflight contributors.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <stdint.h>
#include <string.h>

#include "platform.h"

#if defined(USE_DSHOT_BITBANG) && (defined(STM32F4) || defined(STM32F7))

#include "build/debug.h"
#include "common/log.h"
#include "common/time.h"

#include "drivers/dma.h"
#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/nvic.h"
#include "drivers/timer.h"
#include "drivers/pwm_mapping.h"
#include "drivers/dshot_bitbang.h"

/*
 * Platform-specific macros
 */
#if defined(STM32F4)
    #define BB_GPIO_BSRR_ADDR(gpio)     ((uint32_t)&(gpio)->BSRRL)
#elif defined(STM32F7)
    #define BB_GPIO_BSRR_ADDR(gpio)     ((uint32_t)&(gpio)->BSRR)
#endif

/* TIM DMA source defines - same on both F4 and F7 */
#define BB_TIM_DMA_CC1  0x0200
#define BB_TIM_DMA_CC2  0x0400
#define BB_TIM_DMA_CC3  0x0800
#define BB_TIM_DMA_CC4  0x1000

/* TIM Channel index values (offset in bytes) - same on F4/F7 */
#define BB_TIM_CH1      0x0000
#define BB_TIM_CH2      0x0004
#define BB_TIM_CH3      0x0008
#define BB_TIM_CH4      0x000C

static bbPacer_t bbPacers[BB_MAX_MOTOR_PACERS];
static int usedMotorPacers = 0;

static bbPort_t bbPorts[BB_MAX_MOTOR_PORTS];
static int usedMotorPorts = 0;

static bbMotor_t bbMotors[BB_MAX_MOTORS];

static dshotBitbangStatus_e bbStatus = DSHOT_BB_STATUS_OK;

static uint32_t bbOutputBuffer[BB_DSHOT_BUF_LENGTH * BB_MAX_MOTOR_PORTS];

/*
 * Pacer timer hardware table.
 * Bitbang uses TIM1 or TIM8 CC channels purely as DMA triggers (no pin output).
 *
 * Each entry maps a timer channel to a DMA stream/channel.
 * DMA tag format: DMA_TAG(dma_controller, stream, channel)
 *
 * For STM32F4/F7:
 *   TIM1_CH1 -> DMA2_Stream1_Ch6
 *   TIM1_CH3 -> DMA2_Stream6_Ch6
 *   TIM1_CH4 -> DMA2_Stream4_Ch6
 *   TIM1_CH2 -> DMA2_Stream2_Ch6  (avoid if LED uses DMA2_Stream2)
 *   TIM8_CH2 -> DMA2_Stream3_Ch7
 *   TIM8_CH3 -> DMA2_Stream4_Ch7
 *   TIM8_CH4 -> DMA2_Stream7_Ch7
 *   TIM8_CH1 -> DMA2_Stream2_Ch7  (avoid if conflict with TIM1_CH2)
 */
static const struct {
    TIM_TypeDef *tim;
    uint16_t channel;       /* BB_TIM_CHx offset value */
    uint16_t dmaSource;     /* BB_TIM_DMA_CCx */
    dmaTag_t dmaTag;
} bbPacerTimerHardware[] = {
    { TIM1, BB_TIM_CH1, BB_TIM_DMA_CC1, DMA_TAG(2, 1, 6) },
    { TIM1, BB_TIM_CH3, BB_TIM_DMA_CC3, DMA_TAG(2, 6, 6) },
    { TIM1, BB_TIM_CH4, BB_TIM_DMA_CC4, DMA_TAG(2, 4, 6) },
    { TIM1, BB_TIM_CH2, BB_TIM_DMA_CC2, DMA_TAG(2, 2, 6) },
    { TIM8, BB_TIM_CH2, BB_TIM_DMA_CC2, DMA_TAG(2, 3, 7) },
    { TIM8, BB_TIM_CH3, BB_TIM_DMA_CC3, DMA_TAG(2, 4, 7) },
    { TIM8, BB_TIM_CH4, BB_TIM_DMA_CC4, DMA_TAG(2, 7, 7) },
    { TIM8, BB_TIM_CH1, BB_TIM_DMA_CC1, DMA_TAG(2, 2, 7) },
};
#define BB_PACER_TABLE_SIZE (sizeof(bbPacerTimerHardware) / sizeof(bbPacerTimerHardware[0]))

/*
 * DMA buffer encoding for DSHOT bitbang.
 *
 * Each DSHOT bit is encoded as 3 timer ticks in the DMA output buffer:
 *   [0] SET:    All motor pins HIGH (set mask in BSRR)
 *   [1] MIDDLE: bit=0 → pins LOW (reset mask); bit=1 → no change
 *   [2] RESET:  All motor pins LOW (reset mask in BSRR)
 *
 * 16 bits × 3 ticks + 3 hold ticks = 51 uint32_t per GPIO port.
 * Multiple motors on the same port share the same buffer, each
 * controlling its own bit position in the BSRR word.
 */

static void bbOutputDataInit(uint32_t *buffer, uint16_t portMask, bool inverted)
{
    uint32_t resetMask;
    uint32_t setMask;

    if (inverted) {
        resetMask = portMask;
        setMask = (portMask << 16);
    } else {
        resetMask = (portMask << 16);
        setMask = portMask;
    }

    for (int symbolIdx = 0; symbolIdx < BB_DSHOT_FRAME_BITS; symbolIdx++) {
        buffer[symbolIdx * BB_DSHOT_STATE_PER_SYMBOL + 0] |= setMask;
        /* buffer[symbolIdx * BB_DSHOT_STATE_PER_SYMBOL + 1] is middle, cleared per-frame */
        buffer[symbolIdx * BB_DSHOT_STATE_PER_SYMBOL + 2] |= resetMask;
    }

    int holdIdx = BB_DSHOT_FRAME_BITS * BB_DSHOT_STATE_PER_SYMBOL;
    buffer[holdIdx + 0] |= resetMask;
    buffer[holdIdx + 1] = 0;
    buffer[holdIdx + 2] = 0;
}

static void bbOutputDataSet(uint32_t *buffer, int pinNumber, uint16_t value, bool inverted)
{
    uint32_t middleBit;

    if (inverted) {
        middleBit = (1 << (pinNumber + 0));
    } else {
        middleBit = (1 << (pinNumber + 16));
    }

    for (int pos = 0; pos < 16; pos++) {
        if (!(value & 0x8000)) {
            buffer[pos * 3 + 1] |= middleBit;
        }
        value <<= 1;
    }
}

static void bbOutputDataClear(uint32_t *buffer)
{
    for (int bitpos = 0; bitpos < 16; bitpos++) {
        buffer[bitpos * 3 + 1] = 0;
    }
}

static uint32_t bbGetDshotBaseFrequency(motorPwmProtocolTypes_e proto)
{
    switch (proto) {
    case PWM_TYPE_DSHOT600:
        return BB_DSHOT600_SYMBOL_RATE * BB_DSHOT_STATE_PER_SYMBOL;
    case PWM_TYPE_DSHOT300:
        return BB_DSHOT300_SYMBOL_RATE * BB_DSHOT_STATE_PER_SYMBOL;
    default:
    case PWM_TYPE_DSHOT150:
        return BB_DSHOT150_SYMBOL_RATE * BB_DSHOT_STATE_PER_SYMBOL;
    }
}

static void bbDMAIrqHandler(DMA_t descriptor)
{
    bbPort_t *bbPort = (bbPort_t *)descriptor->userParam;

    /* Disable DMA stream and timer DMA request */
    bbPort->dma->ref->CR &= ~(uint32_t)DMA_SxCR_EN;
    bbPort->pacerTim->DIER &= ~bbPort->dmaSource;

    DMA_CLEAR_FLAG(descriptor, DMA_IT_TCIF);
}

static bbPort_t *bbFindMotorPort(int portIndex)
{
    for (int i = 0; i < usedMotorPorts; i++) {
        if (bbPorts[i].portIndex == portIndex) {
            return &bbPorts[i];
        }
    }
    return NULL;
}

static bool bbAllocatePacerChannel(bbPort_t *bbPort)
{
    for (unsigned i = 0; i < BB_PACER_TABLE_SIZE; i++) {
        dmaTag_t tag = bbPacerTimerHardware[i].dmaTag;
        DMA_t dma = dmaGetByTag(tag);

        if (dma == NULL || dmaGetOwner(dma) != OWNER_FREE) {
            continue;
        }

        /* Check this specific timer+channel isn't already used by another port */
        bool channelConflict = false;
        for (int p = 0; p < usedMotorPorts; p++) {
            if (bbPorts[p].pacerTim == bbPacerTimerHardware[i].tim &&
                bbPorts[p].pacerChannel == bbPacerTimerHardware[i].channel) {
                channelConflict = true;
                break;
            }
        }
        if (channelConflict) {
            continue;
        }

        bbPort->pacerTim = bbPacerTimerHardware[i].tim;
        bbPort->pacerChannel = bbPacerTimerHardware[i].channel;
        bbPort->dma = dma;
        bbPort->dmaChannel = dmaGetChannelByTag(tag);
        bbPort->dmaSource = bbPacerTimerHardware[i].dmaSource;
        bbPort->dmaTag = tag;

        /* Register or update pacer timer entry */
        bool pacerFound = false;
        for (int j = 0; j < usedMotorPacers; j++) {
            if (bbPacers[j].tim == bbPort->pacerTim) {
                bbPacers[j].dmaSources |= bbPort->dmaSource;
                pacerFound = true;
                break;
            }
        }
        if (!pacerFound && usedMotorPacers < BB_MAX_MOTOR_PACERS) {
            bbPacers[usedMotorPacers].tim = bbPort->pacerTim;
            bbPacers[usedMotorPacers].dmaSources = bbPort->dmaSource;
            usedMotorPacers++;
        }

        dmaInit(bbPort->dma, OWNER_MOTOR, RESOURCE_OUTPUT);
        return true;
    }

    return false;
}

static bbPort_t *bbAllocateMotorPort(int portIndex)
{
    if (usedMotorPorts >= BB_MAX_MOTOR_PORTS) {
        bbStatus = DSHOT_BB_STATUS_TOO_MANY_PORTS;
        return NULL;
    }

    bbPort_t *bbPort = &bbPorts[usedMotorPorts];
    bbPort->portIndex = portIndex;

    if (!bbAllocatePacerChannel(bbPort)) {
        bbStatus = DSHOT_BB_STATUS_NO_PACER;
        return NULL;
    }

    usedMotorPorts++;
    return bbPort;
}

static void bbTimerInit(bbPort_t *bbPort, motorPwmProtocolTypes_e proto)
{
    TIM_TypeDef *tim = bbPort->pacerTim;
    uint32_t timClk = timerClock(tim);
    uint32_t outputFreq = bbGetDshotBaseFrequency(proto);

    bbPort->outputARR = timClk / outputFreq - 1;

    /* Enable timer clock */
    uint8_t timId = timer2id(tim);
    RCC_ClockCmd(timerDefinitions[timId].rcc, ENABLE);

    /* Direct register configuration - identical on F4/F7 */
    tim->CR1 = 0;
    tim->PSC = 0;
    tim->ARR = bbPort->outputARR;
    tim->CNT = 0;
    tim->CR1 = TIM_CR1_ARPE;

    /* Configure the OC channel to generate DMA requests (PWM1 mode) */
    /* We just need the CC DMA trigger, no actual pin output */
    volatile uint32_t *ccmr;
    uint32_t ocShift;
    volatile uint32_t *ccr;

    switch (bbPort->pacerChannel) {
    case BB_TIM_CH1:
        ccmr = &tim->CCMR1; ocShift = 0; ccr = &tim->CCR1; break;
    case BB_TIM_CH2:
        ccmr = &tim->CCMR1; ocShift = 8; ccr = &tim->CCR2; break;
    case BB_TIM_CH3:
        ccmr = &tim->CCMR2; ocShift = 0; ccr = &tim->CCR3; break;
    default: /* BB_TIM_CH4 */
        ccmr = &tim->CCMR2; ocShift = 8; ccr = &tim->CCR4; break;
    }

    /* PWM mode 1: OC active while CNT < CCR */
    *ccmr = (*ccmr & ~(0xFF << ocShift)) | ((TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE) << ocShift);
    *ccr = 0;

    /* Enable CC output */
    uint32_t ccerBit = TIM_CCER_CC1E << (bbPort->pacerChannel / 4 * 4);
    tim->CCER |= ccerBit;

    /* For advanced timers (TIM1/TIM8), enable main output */
    if (tim == TIM1 || tim == TIM8) {
        tim->BDTR |= TIM_BDTR_MOE;
    }

    /* Enable timer */
    tim->CR1 |= TIM_CR1_CEN;
}

static void bbDmaInit(bbPort_t *bbPort)
{
    DMA_Stream_TypeDef *stream = bbPort->dma->ref;

    dmaEnableClock(bbPort->dma);

    /* Disable stream before configuration */
    stream->CR &= ~(uint32_t)DMA_SxCR_EN;
    while (stream->CR & DMA_SxCR_EN) {}

    /* Configure DMA stream via direct register access (identical on F4/F7) */
    stream->CR =
        bbPort->dmaChannel |                    /* Channel selection */
        DMA_SxCR_DIR_0 |                        /* Memory to peripheral */
        DMA_SxCR_MINC |                         /* Memory increment */
        DMA_SxCR_PSIZE_1 |                      /* Peripheral size 32-bit */
        DMA_SxCR_MSIZE_1 |                      /* Memory size 32-bit */
        DMA_SxCR_PL_1 |                         /* High priority */
        DMA_SxCR_TCIE;                          /* Transfer complete interrupt */

    stream->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;   /* FIFO enable, 1/2 threshold */

    stream->PAR = BB_GPIO_BSRR_ADDR(bbPort->gpio);
    stream->M0AR = (uint32_t)bbPort->portOutputBuffer;
    stream->NDTR = bbPort->portOutputCount;

    dmaSetHandler(bbPort->dma, bbDMAIrqHandler, NVIC_PRIO_TIMER_DMA, (uint32_t)bbPort);
}

static bool bbMotorConfig(IO_t io, ioTag_t tag, uint8_t motorIndex, motorPwmProtocolTypes_e proto)
{
    if (!io) {
        return false;
    }

    int pinIndex = IO_GPIOPinIdx(io);
    int portIndex = IO_GPIOPortIdx(io);

    bbPort_t *bbPort = bbFindMotorPort(portIndex);

    if (!bbPort) {
        bbPort = bbAllocateMotorPort(portIndex);

        if (!bbPort) {
            return false;
        }

        bbPort->gpio = IO_GPIO(io);
        bbPort->portOutputCount = BB_DSHOT_BUF_LENGTH;
        bbPort->portOutputBuffer = &bbOutputBuffer[(bbPort - bbPorts) * BB_DSHOT_BUF_LENGTH];

        bbTimerInit(bbPort, proto);
        bbDmaInit(bbPort);
    }

    bbMotors[motorIndex].pinIndex = pinIndex;
    bbMotors[motorIndex].portIndex = portIndex;
    bbMotors[motorIndex].ioTag = tag;
    bbMotors[motorIndex].io = io;
    bbMotors[motorIndex].bbPort = bbPort;

    IOInit(io, OWNER_MOTOR, RESOURCE_OUTPUT, motorIndex);
#if defined(STM32F4)
    bbMotors[motorIndex].iocfg = IO_CONFIG(GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_UP);
#elif defined(STM32F7)
    bbMotors[motorIndex].iocfg = IO_CONFIG(GPIO_MODE_OUTPUT_PP, GPIO_SPEED_FREQ_LOW, GPIO_PULLUP);
#endif
    IOConfigGPIO(io, bbMotors[motorIndex].iocfg);
    IOHi(io);

    bbOutputDataInit(bbPort->portOutputBuffer, (1 << pinIndex), false);

    bbMotors[motorIndex].configured = true;
    bbMotors[motorIndex].enabled = true;

    return true;
}

bool dshotBitbangInit(uint8_t motorCount)
{
    memset(bbOutputBuffer, 0, sizeof(bbOutputBuffer));
    memset(bbPacers, 0, sizeof(bbPacers));
    memset(bbPorts, 0, sizeof(bbPorts));
    memset(bbMotors, 0, sizeof(bbMotors));
    usedMotorPacers = 0;
    usedMotorPorts = 0;
    bbStatus = DSHOT_BB_STATUS_OK;

    motorPwmProtocolTypes_e proto = motorConfig()->motorPwmProtocol;

    int motorsConfigured = 0;
    for (int idx = 0; idx < dshotBitbangMotorPinCount && motorsConfigured < motorCount; idx++) {
        ioTag_t tag = dshotBitbangMotorPins[idx];
        IO_t io = IOGetByTag(tag);
        if (!io) {
            continue;
        }

        if (!bbMotorConfig(io, tag, motorsConfigured, proto)) {
            LOG_ERROR(PWM, "Bitbang motor %d init failed", motorsConfigured);
            return false;
        }

        LOG_INFO(PWM, "Bitbang motor %d on port %d pin %d",
                 motorsConfigured, IO_GPIOPortIdx(io), IO_GPIOPinIdx(io));
        motorsConfigured++;
    }

    if (motorsConfigured < motorCount) {
        LOG_ERROR(PWM, "Not enough motor pins for bitbang (%d/%d)", motorsConfigured, motorCount);
        return false;
    }

    LOG_INFO(PWM, "DSHOT Bitbang initialized: %d motors, %d ports, %d pacers",
             motorsConfigured, usedMotorPorts, usedMotorPacers);

    return true;
}

static uint16_t bbPrepareDshotPacket(uint16_t value, bool requestTelemetry)
{
    uint16_t packet = (value << 1) | (requestTelemetry ? 1 : 0);

    int csum = 0;
    int csum_data = packet;
    for (int i = 0; i < 3; i++) {
        csum ^= csum_data;
        csum_data >>= 4;
    }
    csum &= 0xf;
    return (packet << 4) | csum;
}

void dshotBitbangUpdateComplete(uint8_t motorCount, uint16_t *motorValues, bool *requestTelemetry)
{
    /* Clear middle bits from previous frame */
    for (int i = 0; i < usedMotorPorts; i++) {
        bbOutputDataClear(bbPorts[i].portOutputBuffer);
    }

    /* Encode DSHOT packets into DMA buffers */
    for (int index = 0; index < motorCount; index++) {
        bbMotor_t *bbmotor = &bbMotors[index];

        if (!bbmotor->configured) {
            continue;
        }

        uint16_t packet = bbPrepareDshotPacket(motorValues[index], requestTelemetry[index]);
        requestTelemetry[index] = false;

        bbOutputDataSet(bbmotor->bbPort->portOutputBuffer, bbmotor->pinIndex, packet, false);
    }

    /* Reload and start DMA for each GPIO port */
    for (int i = 0; i < usedMotorPorts; i++) {
        bbPort_t *bbPort = &bbPorts[i];
        DMA_Stream_TypeDef *stream = bbPort->dma->ref;

        stream->CR &= ~(uint32_t)DMA_SxCR_EN;
        while (stream->CR & DMA_SxCR_EN) {}

        stream->NDTR = bbPort->portOutputCount;
        stream->M0AR = (uint32_t)bbPort->portOutputBuffer;

        stream->CR |= DMA_SxCR_EN;
    }

    /* Enable timer DMA requests for all pacers */
    for (int i = 0; i < usedMotorPacers; i++) {
        bbPacer_t *bbPacer = &bbPacers[i];
        bbPacer->tim->DIER |= bbPacer->dmaSources;
    }
}

dshotBitbangStatus_e dshotBitbangGetStatus(void)
{
    return bbStatus;
}

ioTag_t dshotBitbangGetMotorPinTag(int motorIndex)
{
    if (motorIndex < BB_MAX_MOTORS && bbMotors[motorIndex].configured) {
        return bbMotors[motorIndex].ioTag;
    }
    return IOTAG_NONE;
}

#endif
