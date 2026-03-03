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

#ifdef USE_DSHOT_BITBANG
#include "drivers/dshot_bitbang.h"

/*
 * Bitbang DSHOT motor pins (S1-S8).
 * Motors use TIM1 as pacer timer; pins are driven as pure GPIO via DMA->BSRR.
 * Grouped by GPIO port for DMA efficiency:
 *   GPIOA: PA8(S7), PA9(S3), PA10(S4) -> DMA2_Stream1 (TIM1_CH1)
 *   GPIOB: PB0(S8), PB14(S5), PB15(S6) -> DMA2_Stream4 (TIM1_CH4)
 *   GPIOC: PC8(S1), PC9(S2) -> DMA2_Stream6 (TIM1_CH3)
 */
const ioTag_t dshotBitbangMotorPins[] = {
    IO_TAG(PC8),    // S1
    IO_TAG(PC9),    // S2
    IO_TAG(PA9),    // S3
    IO_TAG(PA10),   // S4
    IO_TAG(PB14),   // S5
    IO_TAG(PB15),   // S6
    IO_TAG(PA8),    // S7
    IO_TAG(PB0),    // S8
};
const int dshotBitbangMotorPinCount = sizeof(dshotBitbangMotorPins) / sizeof(dshotBitbangMotorPins[0]);
#endif

timerHardware_t timerHardware[] = {
    // Servos: independent timers, no frequency conflict with LED or bitbang pacer
    DEF_TIM(TIM13,  CH1,  PA6,  TIM_USE_SERVO,       0, 0), // S9  - SERVO1 (50Hz)
    DEF_TIM(TIM3,   CH4,  PB1,  TIM_USE_SERVO,       0, 0), // S10 - SERVO2 (50Hz)

    // LED strip on TIM8_CH1N (PA7) - DMA2_Stream2_Ch7, ~800kHz WS2812
    DEF_TIM(TIM8,   CH1N, PA7,  TIM_USE_LED,         0, 0), // LED Strip

    // Softserial
    DEF_TIM(TIM5,   CH3,  PA2,  TIM_USE_ANY,         0, 0), // Softserial1_Tx
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
