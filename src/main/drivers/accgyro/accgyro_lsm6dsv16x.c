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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/accgyro/accgyro_lsm6dsv16x.h"

#if defined(USE_IMU_LSM6DSV16X)

typedef struct __attribute__ ((__packed__)) lsm6dsv16xContextData_s {
    uint16_t    chipMagicNumber;
    uint8_t     lastReadStatus;
    uint8_t     __padding_dummy;
    uint8_t     accRaw[6];
    uint8_t     gyroRaw[6];
} lsm6dsv16xContextData_t;

static void lsm6dsv16xWriteRegister(const busDevice_t *dev, lsm6dsv16xRegister_e reg, uint8_t value, unsigned delayMs)
{
    busWrite(dev, reg, value);
    if (delayMs) {
        delay(delayMs);
    }
}

static uint8_t lsm6dsv16xGetLpf1Bandwidth(gyroDev_t *gyro)
{
    // LPF1 bandwidth selection based on hardware LPF setting
    // At 8kHz ODR: 0=~293Hz, 1=~222Hz, 2=~162Hz, 3=~424Hz
    switch (gyro->lpf) {
        case GYRO_LPF_256HZ:  // NORMAL - closest to MPU6000's 250Hz
            return LSM6DSV16X_CTRL6_LPF1_BW_0;  // ~293Hz @8kHz
        case GYRO_LPF_188HZ:  // OPTION_1
            return LSM6DSV16X_CTRL6_LPF1_BW_2;  // ~162Hz @8kHz
        case GYRO_LPF_98HZ:   // OPTION_2
            return LSM6DSV16X_CTRL6_LPF1_BW_1;  // ~222Hz @8kHz
        case GYRO_LPF_42HZ:
        case GYRO_LPF_20HZ:
        case GYRO_LPF_10HZ:
        case GYRO_LPF_5HZ:
            return LSM6DSV16X_CTRL6_LPF1_BW_4;  // ~106Hz @8kHz
        default:
            return LSM6DSV16X_CTRL6_LPF1_BW_3;  // ~424Hz @8kHz (widest)
    }
}

static void lsm6dsv16xConfig(gyroDev_t *gyro)
{
    busDevice_t *dev = gyro->busDev;

    // Set sample rate - LSM6DSV16X runs at 8kHz in high-accuracy mode
    gyro->sampleRateIntervalUs = 125;  // 8kHz = 125us

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    // Software reset
    lsm6dsv16xWriteRegister(dev, LSM6DSV16X_REG_CTRL3, LSM6DSV16X_CTRL3_SW_RESET, 50);

    // Wait for reset to complete (with timeout protection)
    uint8_t ctrl3;
    int resetTimeout = 100;  // 100ms max wait
    do {
        delay(1);
        busRead(dev, LSM6DSV16X_REG_CTRL3, &ctrl3);
        if (--resetTimeout <= 0) {
            break;  // Timeout, avoid infinite loop
        }
    } while (ctrl3 & LSM6DSV16X_CTRL3_SW_RESET);

    // Additional stabilization delay after reset (datasheet: gyro needs 30ms turn-on time)
    delay(35);

    // Auto-increment address and block data update
    lsm6dsv16xWriteRegister(dev, LSM6DSV16X_REG_CTRL3, 
                            LSM6DSV16X_CTRL3_IF_INC | LSM6DSV16X_CTRL3_BDU, 1);

    // Select high-accuracy ODR mode 1 (8kHz max)
    lsm6dsv16xWriteRegister(dev, LSM6DSV16X_REG_HAODR_CFG, LSM6DSV16X_HAODR_MODE1, 1);

    // Configure accelerometer: 16G full scale
    lsm6dsv16xWriteRegister(dev, LSM6DSV16X_REG_CTRL8, LSM6DSV16X_CTRL8_FS_16G, 1);

    // Configure accelerometer: HIGH_ACCURACY mode + 1kHz ODR (1000Hz in HAODR mode 1 = 0x09)
    // CTRL1 = (OP_MODE_XL << 4) | ODR_XL = (1 << 4) | 9 = 0x19
    lsm6dsv16xWriteRegister(dev, LSM6DSV16X_REG_CTRL1, 0x19, 1);

    // Configure gyroscope: HIGH_ACCURACY mode + 8kHz ODR
    // CTRL2 = (OP_MODE_G << 4) | ODR_G = (1 << 4) | 12 = 0x1C
    lsm6dsv16xWriteRegister(dev, LSM6DSV16X_REG_CTRL2, 0x1C, 1);

    // Configure gyroscope: 2000 deg/s full-scale + LPF1 bandwidth
    uint8_t ctrl6Value = LSM6DSV16X_CTRL6_FS_2000DPS | lsm6dsv16xGetLpf1Bandwidth(gyro);
    lsm6dsv16xWriteRegister(dev, LSM6DSV16X_REG_CTRL6, ctrl6Value, 1);

    // Enable gyro LPF1 filter
    lsm6dsv16xWriteRegister(dev, LSM6DSV16X_REG_CTRL7, LSM6DSV16X_CTRL7_LPF1_G_EN, 1);

    // Generate pulse on interrupt line (pulsed mode)
    lsm6dsv16xWriteRegister(dev, LSM6DSV16X_REG_CTRL4, LSM6DSV16X_CTRL4_DRDY_PULSED, 1);

    // Enable gyro data ready interrupt on INT1
    lsm6dsv16xWriteRegister(dev, LSM6DSV16X_REG_INT1_CTRL, LSM6DSV16X_INT1_DRDY_G, 1);

    busSetSpeed(dev, BUS_SPEED_FAST);
}

static bool lsm6dsv16xDetect(busDevice_t *dev)
{
    uint8_t tmp;
    uint8_t attemptsRemaining = 5;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    do {
        delay(150);
        busRead(dev, LSM6DSV16X_REG_WHO_AM_I, &tmp);

        if (tmp == LSM6DSV16X_CHIP_ID) {
            return true;
        }
    } while (attemptsRemaining--);

    return false;
}

static void lsm6dsv16xGyroInit(gyroDev_t *gyro)
{
    lsm6dsv16xConfig(gyro);
}

static void lsm6dsv16xAccInit(accDev_t *acc)
{
    // Sensor is configured during gyro init
    acc->acc_1G = 512 * 4;   // 16G sensor scale
}

static bool lsm6dsv16xAccRead(accDev_t *acc)
{
    uint8_t data[6];

    const bool ack = busReadBuf(acc->busDev, LSM6DSV16X_REG_OUTX_L_A, data, 6);
    if (!ack) {
        return false;
    }

    acc->ADCRaw[X] = (float)int16_val_little_endian(data, 0);
    acc->ADCRaw[Y] = (float)int16_val_little_endian(data, 1);
    acc->ADCRaw[Z] = (float)int16_val_little_endian(data, 2);

    return true;
}

static bool lsm6dsv16xGyroRead(gyroDev_t *gyro)
{
    uint8_t data[6];

    const bool ack = busReadBuf(gyro->busDev, LSM6DSV16X_REG_OUTX_L_G, data, 6);
    if (!ack) {
        return false;
    }

    gyro->gyroADCRaw[X] = (float)int16_val_little_endian(data, 0);
    gyro->gyroADCRaw[Y] = (float)int16_val_little_endian(data, 1);
    gyro->gyroADCRaw[Z] = (float)int16_val_little_endian(data, 2);

    return true;
}

bool lsm6dsv16xGyroDetect(gyroDev_t *gyro)
{
    gyro->busDev = busDeviceInit(BUSTYPE_SPI, DEVHW_LSM6DSV16X, gyro->imuSensorToUse, OWNER_MPU);
    if (gyro->busDev == NULL) {
        return false;
    }

    if (!lsm6dsv16xDetect(gyro->busDev)) {
        busDeviceDeInit(gyro->busDev);
        return false;
    }

    lsm6dsv16xContextData_t *ctx = busDeviceGetScratchpadMemory(gyro->busDev);
    ctx->chipMagicNumber = 0xD7;  // Magic number for LSM6DSV16X

    gyro->initFn = lsm6dsv16xGyroInit;
    gyro->readFn = lsm6dsv16xGyroRead;
    gyro->intStatusFn = gyroCheckDataReady;
    // From datasheet: G_So is 70mdps/LSB for FS = ±2000 dps
    gyro->scale = 0.070f;

    return true;
}

bool lsm6dsv16xAccDetect(accDev_t *acc)
{
    acc->busDev = busDeviceOpen(BUSTYPE_SPI, DEVHW_LSM6DSV16X, acc->imuSensorToUse);
    if (acc->busDev == NULL) {
        return false;
    }

    lsm6dsv16xContextData_t *ctx = busDeviceGetScratchpadMemory(acc->busDev);
    if (ctx->chipMagicNumber != 0xD7) {
        return false;
    }

    acc->initFn = lsm6dsv16xAccInit;
    acc->readFn = lsm6dsv16xAccRead;
    acc->accAlign = acc->busDev->param;

    return true;
}

#endif // USE_IMU_LSM6DSV16X

