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
 * DMA stream allocation (optimized, no conflicts):
 *
 * Motor 1 (PC9  / TIM8_CH4): dmaopt 0 → DMA2_Stream_7_Ch7
 * Motor 2 (PA8  / TIM1_CH1): dmaopt 1 → DMA2_Stream_1_Ch6
 * Motor 3 (PA9  / TIM1_CH2): dmaopt 1 → DMA2_Stream_2_Ch6
 * Motor 4 (PA10 / TIM1_CH3): dmaopt 0 → DMA2_Stream_6_Ch0
 * LED     (PB1  / TIM3_CH4): dmaopt 0 → DMA1_Stream_2_Ch5
 *
 * Note: TIM1_CH1/CH2/CH3 dmaopt 0 all map to DMA2_Stream_6 which would
 * conflict. CH1 uses dmaopt 1 (Stream_1), CH2 uses dmaopt 1 (Stream_2),
 * CH3 keeps dmaopt 0 (Stream_6) — all unique streams.
 */

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM8,  CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 0),  // S1 - DMA2_Stream_7
    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_OUTPUT_AUTO, 0, 1),  // S2 - DMA2_Stream_1
    DEF_TIM(TIM1,  CH2, PA9,  TIM_USE_OUTPUT_AUTO, 0, 1),  // S3 - DMA2_Stream_2
    DEF_TIM(TIM1,  CH3, PA10, TIM_USE_OUTPUT_AUTO, 0, 0),  // S4 - DMA2_Stream_6

    DEF_TIM(TIM3,  CH4, PB1,  TIM_USE_LED,         0, 0),  // LED - DMA1_Stream_2
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
