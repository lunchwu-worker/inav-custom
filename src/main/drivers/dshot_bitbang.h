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

#pragma once

#include "platform.h"

#ifdef USE_DSHOT_BITBANG

#include <stdbool.h>
#include <stdint.h>

#include "drivers/io_types.h"
#include "drivers/dma.h"
#include "drivers/timer.h"

#define BB_MAX_MOTOR_PACERS     4
#define BB_MAX_MOTOR_PORTS      4
#define BB_MAX_MOTORS           MAX_MOTORS

#define BB_DSHOT_STATE_PER_SYMBOL       3
#define BB_DSHOT_BIT_HOLD_STATES        3
#define BB_DSHOT_FRAME_BITS             16
#define BB_DSHOT_BUF_LENGTH             ((BB_DSHOT_FRAME_BITS * BB_DSHOT_STATE_PER_SYMBOL) + BB_DSHOT_BIT_HOLD_STATES)

#define BB_DSHOT600_SYMBOL_RATE     (600 * 1000)
#define BB_DSHOT300_SYMBOL_RATE     (300 * 1000)
#define BB_DSHOT150_SYMBOL_RATE     (150 * 1000)

typedef enum {
    DSHOT_BB_STATUS_OK,
    DSHOT_BB_STATUS_MOTOR_PIN_CONFLICT,
    DSHOT_BB_STATUS_NO_PACER,
    DSHOT_BB_STATUS_TOO_MANY_PORTS,
} dshotBitbangStatus_e;

typedef struct bbPacer_s {
    TIM_TypeDef *tim;
    uint16_t dmaSources;
} bbPacer_t;

typedef struct bbPort_s {
    int portIndex;
    GPIO_TypeDef *gpio;

    TIM_TypeDef *pacerTim;
    uint16_t pacerChannel;
    uint16_t dmaSource;
    dmaTag_t dmaTag;
    DMA_t dma;
    uint32_t dmaChannel;

    uint16_t outputARR;

    uint32_t *portOutputBuffer;
    uint32_t portOutputCount;
} bbPort_t;

typedef struct bbMotor_s {
    int pinIndex;
    int portIndex;
    ioTag_t ioTag;
    IO_t io;
    uint32_t iocfg;
    bbPort_t *bbPort;
    bool configured;
    bool enabled;
} bbMotor_t;

extern const ioTag_t dshotBitbangMotorPins[];
extern const int dshotBitbangMotorPinCount;

bool dshotBitbangInit(uint8_t motorCount);
void dshotBitbangUpdateComplete(uint8_t motorCount, uint16_t *motorValues, bool *requestTelemetry);
dshotBitbangStatus_e dshotBitbangGetStatus(void);
ioTag_t dshotBitbangGetMotorPinTag(int motorIndex);

#endif
