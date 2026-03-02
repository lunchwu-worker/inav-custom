/*
 * This file is part of INAV.
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

#include <stdbool.h>
#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM8,   CH3,  PC8,  TIM_USE_OUTPUT_AUTO, 1, 0), // S1 - DMA2_Stream4
    DEF_TIM(TIM8,   CH4,  PC9,  TIM_USE_OUTPUT_AUTO, 1, 0), // S2 - DMA2_Stream7

    DEF_TIM(TIM1,   CH2,  PA9,  TIM_USE_OUTPUT_AUTO, 1, 1), // S3 - DMA2_Stream2
    DEF_TIM(TIM1,   CH3,  PA10, TIM_USE_OUTPUT_AUTO, 1, 0), // S4 - DMA2_Stream6
    DEF_TIM(TIM12,  CH1,  PB14, TIM_USE_OUTPUT_AUTO, 1, 0), // S5 - no DMA (TIM12)
    DEF_TIM(TIM12,  CH2,  PB15, TIM_USE_OUTPUT_AUTO, 1, 0), // S6 - no DMA (TIM12)

    DEF_TIM(TIM1,   CH1,  PA8,  TIM_USE_OUTPUT_AUTO, 1, 1), // S7 - DMA2_Stream1
    DEF_TIM(TIM3,   CH4,  PB1,  TIM_USE_OUTPUT_AUTO, 1, 0), // S8 - DMA1_Stream2

    DEF_TIM(TIM3,   CH1,  PA6,  TIM_USE_OUTPUT_AUTO, 0, 0), // S9  - SERVO1
    DEF_TIM(TIM2,   CH3,  PB10, TIM_USE_OUTPUT_AUTO, 0, 0), // S10 - SERVO2

    DEF_TIM(TIM3,   CH2,  PA7,  TIM_USE_LED,         0, 0), // LED Strip
    DEF_TIM(TIM5,   CH3,  PA2,  TIM_USE_ANY,         0, 0), // Softserial1_Tx
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
