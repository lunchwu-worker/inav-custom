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
#include "drivers/pinio.h"
#include "io/piniobox.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM5, CH4, PA3,  TIM_USE_PPM,         0, 0),   // PPM

    DEF_TIM(TIM8, CH3, PC8,  TIM_USE_OUTPUT_AUTO,  0, 1),   // Motor 1
    DEF_TIM(TIM8, CH4, PC9,  TIM_USE_OUTPUT_AUTO,  0, 0),   // Motor 2
    DEF_TIM(TIM1, CH2, PA9,  TIM_USE_OUTPUT_AUTO,  0, 1),   // Motor 3
    DEF_TIM(TIM1, CH3, PA10, TIM_USE_OUTPUT_AUTO,  0, 0),   // Motor 4
    DEF_TIM(TIM1, CH1, PA8,  TIM_USE_OUTPUT_AUTO,  0, 1),   // Motor 5
    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_OUTPUT_AUTO,  0, 0),   // Motor 6
    DEF_TIM(TIM12, CH1, PB14, TIM_USE_OUTPUT_AUTO, 0, 0),   // Motor 7 / Servo
    DEF_TIM(TIM12, CH2, PB15, TIM_USE_OUTPUT_AUTO, 0, 0),   // Motor 8 / Servo

    DEF_TIM(TIM13, CH1, PA6, TIM_USE_OUTPUT_AUTO,  0, 0),   // Servo 1
    DEF_TIM(TIM3, CH4, PB1,  TIM_USE_OUTPUT_AUTO,  0, 0),   // Servo 2

    DEF_TIM(TIM3, CH2, PA7,  TIM_USE_LED,          0, 0),   // LED strip
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
