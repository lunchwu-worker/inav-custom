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
 *
 * Ported from Betaflight
 */

#pragma once

#include "drivers/bus.h"

// LSM6DSV16X Chip ID
#define LSM6DSV16X_CHIP_ID  0x70

// LSM6DSK320X Chip ID. Same register map as LSM6DSV16X, plus a ±320g high-g channel.
#define LSM6DSK320X_CHIP_ID 0x75

// LSM6DSV16X registers
typedef enum {
    LSM6DSV16X_REG_FUNC_CFG_ACCESS  = 0x01,
    LSM6DSV16X_REG_PIN_CTRL         = 0x02,
    LSM6DSV16X_REG_IF_CFG           = 0x03,
    LSM6DSV16X_REG_FIFO_CTRL1       = 0x07,
    LSM6DSV16X_REG_FIFO_CTRL2       = 0x08,
    LSM6DSV16X_REG_FIFO_CTRL3       = 0x09,
    LSM6DSV16X_REG_FIFO_CTRL4       = 0x0A,
    LSM6DSV16X_REG_COUNTER_BDR1     = 0x0B,
    LSM6DSV16X_REG_COUNTER_BDR2     = 0x0C,
    LSM6DSV16X_REG_INT1_CTRL        = 0x0D,
    LSM6DSV16X_REG_INT2_CTRL        = 0x0E,
    LSM6DSV16X_REG_WHO_AM_I         = 0x0F,
    LSM6DSV16X_REG_CTRL1            = 0x10,  // Accelerometer control
    LSM6DSV16X_REG_CTRL2            = 0x11,  // Gyroscope control
    LSM6DSV16X_REG_CTRL3            = 0x12,  // Control register 3
    LSM6DSV16X_REG_CTRL4            = 0x13,  // Control register 4
    LSM6DSV16X_REG_CTRL5            = 0x14,  // Control register 5
    LSM6DSV16X_REG_CTRL6            = 0x15,  // Control register 6 (gyro)
    LSM6DSV16X_REG_CTRL7            = 0x16,  // Control register 7
    LSM6DSV16X_REG_CTRL8            = 0x17,  // Control register 8 (acc)
    LSM6DSV16X_REG_CTRL9            = 0x18,  // Control register 9
    LSM6DSV16X_REG_CTRL10           = 0x19,  // Control register 10
    LSM6DSV16X_REG_CTRL_STATUS      = 0x1A,
    LSM6DSV16X_REG_FIFO_STATUS1     = 0x1B,
    LSM6DSV16X_REG_FIFO_STATUS2     = 0x1C,
    LSM6DSV16X_REG_ALL_INT_SRC      = 0x1D,
    LSM6DSV16X_REG_STATUS           = 0x1E,
    LSM6DSV16X_REG_OUT_TEMP_L       = 0x20,
    LSM6DSV16X_REG_OUT_TEMP_H       = 0x21,
    LSM6DSV16X_REG_OUTX_L_G         = 0x22,  // Gyro X axis LSB
    LSM6DSV16X_REG_OUTX_H_G         = 0x23,  // Gyro X axis MSB
    LSM6DSV16X_REG_OUTY_L_G         = 0x24,  // Gyro Y axis LSB
    LSM6DSV16X_REG_OUTY_H_G         = 0x25,  // Gyro Y axis MSB
    LSM6DSV16X_REG_OUTZ_L_G         = 0x26,  // Gyro Z axis LSB
    LSM6DSV16X_REG_OUTZ_H_G         = 0x27,  // Gyro Z axis MSB
    LSM6DSV16X_REG_OUTX_L_A         = 0x28,  // Acc X axis LSB
    LSM6DSV16X_REG_OUTX_H_A         = 0x29,  // Acc X axis MSB
    LSM6DSV16X_REG_OUTY_L_A         = 0x2A,  // Acc Y axis LSB
    LSM6DSV16X_REG_OUTY_H_A         = 0x2B,  // Acc Y axis MSB
    LSM6DSV16X_REG_OUTZ_L_A         = 0x2C,  // Acc Z axis LSB
    LSM6DSV16X_REG_OUTZ_H_A         = 0x2D,  // Acc Z axis MSB
    LSM6DSV16X_REG_HAODR_CFG        = 0x62,  // High-accuracy ODR config
} lsm6dsv16xRegister_e;

// CTRL1 (Accelerometer) values
typedef enum {
    // ODR settings (bits 3:0) - values for high-accuracy ODR mode 1 (HAODR_SEL = 01)
    LSM6DSV16X_CTRL1_ODR_OFF        = 0x00,
    LSM6DSV16X_CTRL1_ODR_1875HZ     = 0x01,
    LSM6DSV16X_CTRL1_ODR_7_5HZ      = 0x02,
    LSM6DSV16X_CTRL1_ODR_15HZ       = 0x03,
    LSM6DSV16X_CTRL1_ODR_30HZ       = 0x04,
    LSM6DSV16X_CTRL1_ODR_60HZ       = 0x05,
    LSM6DSV16X_CTRL1_ODR_120HZ      = 0x06,
    LSM6DSV16X_CTRL1_ODR_240HZ      = 0x07,
    LSM6DSV16X_CTRL1_ODR_480HZ      = 0x08,
    LSM6DSV16X_CTRL1_ODR_960HZ      = 0x09,
    LSM6DSV16X_CTRL1_ODR_1920HZ     = 0x0A,
    LSM6DSV16X_CTRL1_ODR_3840HZ     = 0x0B,
    LSM6DSV16X_CTRL1_ODR_7680HZ     = 0x0C,
    // OP_MODE (bits 6:4)
    LSM6DSV16X_CTRL1_OP_HIGH_PERF   = 0x00,  // High-performance mode
    LSM6DSV16X_CTRL1_OP_HIGH_ACC    = 0x10,  // High-accuracy mode
} lsm6dsv16xCtrl1Values_e;

// CTRL2 (Gyroscope) values
typedef enum {
    // ODR settings (bits 3:0) - values for high-accuracy ODR mode 1 (HAODR_SEL = 01)
    LSM6DSV16X_CTRL2_ODR_OFF        = 0x00,
    LSM6DSV16X_CTRL2_ODR_1875HZ     = 0x01,
    LSM6DSV16X_CTRL2_ODR_7_5HZ      = 0x02,
    LSM6DSV16X_CTRL2_ODR_15HZ       = 0x03,
    LSM6DSV16X_CTRL2_ODR_30HZ       = 0x04,
    LSM6DSV16X_CTRL2_ODR_60HZ       = 0x05,
    LSM6DSV16X_CTRL2_ODR_120HZ      = 0x06,
    LSM6DSV16X_CTRL2_ODR_240HZ      = 0x07,
    LSM6DSV16X_CTRL2_ODR_480HZ      = 0x08,
    LSM6DSV16X_CTRL2_ODR_960HZ      = 0x09,
    LSM6DSV16X_CTRL2_ODR_1920HZ     = 0x0A,
    LSM6DSV16X_CTRL2_ODR_3840HZ     = 0x0B,
    LSM6DSV16X_CTRL2_ODR_7680HZ     = 0x0C,
    // OP_MODE (bits 6:4)
    LSM6DSV16X_CTRL2_OP_HIGH_PERF   = 0x00,  // High-performance mode
    LSM6DSV16X_CTRL2_OP_HIGH_ACC    = 0x10,  // High-accuracy mode
} lsm6dsv16xCtrl2Values_e;

// CTRL3 values
typedef enum {
    LSM6DSV16X_CTRL3_SW_RESET       = 0x01,  // Software reset
    LSM6DSV16X_CTRL3_IF_INC         = 0x04,  // Auto-increment address
    LSM6DSV16X_CTRL3_BDU            = 0x40,  // Block data update
    LSM6DSV16X_CTRL3_BOOT           = 0x80,  // Reboot memory content
} lsm6dsv16xCtrl3Values_e;

// CTRL4 values
typedef enum {
    LSM6DSV16X_CTRL4_INT2_IN_LH     = 0x01,
    LSM6DSV16X_CTRL4_DRDY_PULSED    = 0x02,  // Data ready pulsed mode
    LSM6DSV16X_CTRL4_INT2_DRDY_TEMP = 0x04,
    LSM6DSV16X_CTRL4_DRDY_MASK      = 0x08,
    LSM6DSV16X_CTRL4_INT2_ON_INT1   = 0x10,
} lsm6dsv16xCtrl4Values_e;

// CTRL6 (Gyroscope full-scale and LPF) values
typedef enum {
    // Full-scale (bits 3:0)
    LSM6DSV16X_CTRL6_FS_125DPS      = 0x00,
    LSM6DSV16X_CTRL6_FS_250DPS      = 0x01,
    LSM6DSV16X_CTRL6_FS_500DPS      = 0x02,
    LSM6DSV16X_CTRL6_FS_1000DPS     = 0x03,
    LSM6DSV16X_CTRL6_FS_2000DPS     = 0x04,
    LSM6DSV16X_CTRL6_FS_4000DPS     = 0x0C,
    // LPF1 bandwidth (bits 6:4) - See datasheet Table 64
    // At 8kHz ODR: 0=~293Hz, 1=~222Hz, 2=~162Hz, 3=~424Hz, 4=~106Hz, 5=~60Hz, 6=~30Hz, 7=~15Hz
    LSM6DSV16X_CTRL6_LPF1_BW_0      = 0x00,  // ~293Hz @8kHz (closest to MPU6000's 250Hz)
    LSM6DSV16X_CTRL6_LPF1_BW_1      = 0x10,  // ~222Hz @8kHz
    LSM6DSV16X_CTRL6_LPF1_BW_2      = 0x20,  // ~162Hz @8kHz
    LSM6DSV16X_CTRL6_LPF1_BW_3      = 0x30,  // ~424Hz @8kHz (widest)
    LSM6DSV16X_CTRL6_LPF1_BW_4      = 0x40,  // ~106Hz @8kHz
    LSM6DSV16X_CTRL6_LPF1_BW_5      = 0x50,  // ~60Hz @8kHz
    LSM6DSV16X_CTRL6_LPF1_BW_6      = 0x60,  // ~30Hz @8kHz
    LSM6DSV16X_CTRL6_LPF1_BW_7      = 0x70,  // ~15Hz @8kHz (narrowest)
} lsm6dsv16xCtrl6Values_e;

// CTRL7 values
typedef enum {
    LSM6DSV16X_CTRL7_LPF1_G_EN      = 0x01,  // Enable gyro LPF1
    LSM6DSV16X_CTRL7_AH_QVAR_EN     = 0x80,
} lsm6dsv16xCtrl7Values_e;

// CTRL8 (Accelerometer full-scale) values
typedef enum {
    // Full-scale (bits 1:0) — low-g channel. LSM6DSK320X also has a separate ±320g high-g channel.
    LSM6DSV16X_CTRL8_FS_2G          = 0x00,
    LSM6DSV16X_CTRL8_FS_4G          = 0x01,
    LSM6DSV16X_CTRL8_FS_8G          = 0x02,
    LSM6DSV16X_CTRL8_FS_16G         = 0x03,
} lsm6dsv16xCtrl8Values_e;

// CTRL9 values
typedef enum {
    LSM6DSV16X_CTRL9_LPF2_XL_EN     = 0x08,  // Enable accelerometer LPF2
} lsm6dsv16xCtrl9Values_e;

// INT1_CTRL values
typedef enum {
    LSM6DSV16X_INT1_DRDY_XL         = 0x01,  // Acc data ready on INT1
    LSM6DSV16X_INT1_DRDY_G          = 0x02,  // Gyro data ready on INT1
    LSM6DSV16X_INT1_FIFO_TH         = 0x08,
    LSM6DSV16X_INT1_FIFO_OVR        = 0x10,
    LSM6DSV16X_INT1_FIFO_FULL       = 0x20,
    LSM6DSV16X_INT1_CNT_BDR         = 0x40,
} lsm6dsv16xInt1CtrlValues_e;

// HAODR_CFG values
typedef enum {
    LSM6DSV16X_HAODR_MODE0          = 0x00,  // Standard ODR values
    LSM6DSV16X_HAODR_MODE1          = 0x01,  // High-accuracy ODR values (8kHz max)
    LSM6DSV16X_HAODR_MODE2          = 0x02,
} lsm6dsv16xHaodrCfgValues_e;

bool lsm6dsv16xAccDetect(accDev_t *acc);
bool lsm6dsv16xGyroDetect(gyroDev_t *gyro);

bool lsm6dsk320xAccDetect(accDev_t *acc);
bool lsm6dsk320xGyroDetect(gyroDev_t *gyro);

