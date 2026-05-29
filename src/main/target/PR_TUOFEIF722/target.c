/*
 * This file is part of INAV Project.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/sensor.h"

/*
 * DMA stream allocation (optimized, all unique, no conflicts):
 *
 * Motor 1 (PC8  / TIM8_CH3): dmaopt 0 → DMA2_Stream_4
 * Motor 2 (PB6  / TIM4_CH1): dmaopt 0 → DMA1_Stream_0
 * Motor 3 (PC9  / TIM8_CH4): dmaopt 0 → DMA2_Stream_7
 * Motor 4 (PB7  / TIM4_CH2): dmaopt 0 → DMA1_Stream_3
 * LED     (PA15 / TIM2_CH1): dmaopt 0 → DMA1_Stream_5
 * ADC1                     :          → DMA2_Stream_0
 *
 * Motors are spread across two timers (TIM8: M1/M3, TIM4: M2/M4), each
 * channel landing on a unique DMA stream — ideal for DSHOT.
 * ADC1 is placed on DMA2_Stream_0 to avoid the M1 DMA2_Stream_4 channel.
 */

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM8, CH3, PC8,  TIM_USE_OUTPUT_AUTO, 0, 0),  // M1 - DMA2_Stream_4
    DEF_TIM(TIM4, CH1, PB6,  TIM_USE_OUTPUT_AUTO, 0, 0),  // M2 - DMA1_Stream_0
    DEF_TIM(TIM8, CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 0),  // M3 - DMA2_Stream_7
    DEF_TIM(TIM4, CH2, PB7,  TIM_USE_OUTPUT_AUTO, 0, 0),  // M4 - DMA1_Stream_3

    DEF_TIM(TIM2, CH1, PA15, TIM_USE_LED,         0, 0),  // LED strip - DMA1_Stream_5
    DEF_TIM(TIM1, CH1, PA8,  TIM_USE_ANY,         0, 0),  // Camera control
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
