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
#include "drivers/pinio.h"
#include "drivers/sensor.h"

/*
 * Pad order follows the Betaflight target: M1..M7 first, then the two servo
 * pads, then M8. Betaflight reaches all eight motor pads with bitbang DShot,
 * INAV needs a hardware DMA channel per output, hence the differences below.
 *
 * DMA streams (ADC1 owns DMA2_Stream0):
 *   PC8  TIM8_CH3  DMA2_Stream4      PC9  TIM8_CH4  DMA2_Stream7
 *   PA9  TIM1_CH2  DMA2_Stream2      PA10 TIM1_CH3  DMA2_Stream6
 *   PA8  TIM1_CH1  DMA2_Stream1      PB10 TIM2_CH3  DMA1_Stream1
 *   PB1  TIM3_CH4  DMA1_Stream2      PA7  TIM3_CH2  DMA1_Stream5 (WS2811)
 *
 * PB14/PB15 only reach TIM12, which has no DMA on F7: those two pads are
 * analog PWM/servo outputs, they cannot run DShot.
 *
 * PB1 (M8) shares TIM3 with the WS2811 pin, so it is the last entry: enabling
 * the LED strip feature drops it and leaves outputs 1..9 unchanged.
 */
timerHardware_t timerHardware[] = {
    DEF_TIM(TIM8,  CH3, PC8,  TIM_USE_OUTPUT_AUTO, 0, 0), // M1
    DEF_TIM(TIM8,  CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 0), // M2
    DEF_TIM(TIM1,  CH2, PA9,  TIM_USE_OUTPUT_AUTO, 0, 1), // M3
    DEF_TIM(TIM1,  CH3, PA10, TIM_USE_OUTPUT_AUTO, 0, 1), // M4
    DEF_TIM(TIM12, CH1, PB14, TIM_USE_OUTPUT_AUTO, 0, 0), // M5, no DMA
    DEF_TIM(TIM12, CH2, PB15, TIM_USE_OUTPUT_AUTO, 0, 0), // M6, no DMA
    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_OUTPUT_AUTO, 0, 1), // M7
    DEF_TIM(TIM13, CH1, PA6,  TIM_USE_OUTPUT_AUTO, 0, 0), // SERVO1, no DMA
    DEF_TIM(TIM2,  CH3, PB10, TIM_USE_OUTPUT_AUTO, 0, 0), // SERVO2
    DEF_TIM(TIM3,  CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 0), // M8, shares TIM3 with the LED strip
    DEF_TIM(TIM3,  CH2, PA7,  TIM_USE_LED,         0, 0), // WS2811
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
