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
 * Driver for Invensense ICM-42607-P / ICM-42670-P IMU
 *
 * ICM-42607-P and ICM-42670-P share the same register map.
 * Datasheet reference & implementation based on:
 *   - Betaflight ICM42607 driver
 *   - PX4 ICM42670P driver
 *   - TDK InvenSense ICM-42607-P / ICM-42670-P datasheet
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"
#include "common/log.h"

#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/accgyro/accgyro_icm42607.h"

#if defined(USE_IMU_ICM42607)

/* ===== Bank 0 register addresses ===== */
#define ICM42607_RA_MCLK_RDY                       0x00
#define ICM42607_RA_SIGNAL_PATH_RESET               0x02
#define ICM42607_RA_INT_CONFIG                      0x06
#define ICM42607_RA_TEMP_DATA1                      0x09
#define ICM42607_RA_TEMP_DATA0                      0x0A
#define ICM42607_RA_ACCEL_DATA_X1                   0x0B
#define ICM42607_RA_ACCEL_DATA_X0                   0x0C
#define ICM42607_RA_GYRO_DATA_X1                    0x11
#define ICM42607_RA_GYRO_DATA_X0                    0x12
#define ICM42607_RA_INT_STATUS                      0x3A
#define ICM42607_RA_PWR_MGMT0                       0x1F
#define ICM42607_RA_GYRO_CONFIG0                    0x20
#define ICM42607_RA_ACCEL_CONFIG0                   0x21
#define ICM42607_RA_GYRO_CONFIG1                    0x23
#define ICM42607_RA_ACCEL_CONFIG1                   0x24
#define ICM42607_RA_INT_SOURCE0                     0x2B
#define ICM42607_RA_INTF_CONFIG0                    0x35
#define ICM42607_RA_INTF_CONFIG1                    0x36
#define ICM42607_RA_WHO_AM_I                        0x75

/* MREG (Multi-bank Register) access registers */
#define ICM42607_RA_BLK_SEL_W                       0x79
#define ICM42607_RA_MADDR_W                         0x7A
#define ICM42607_RA_M_W                             0x7B
#define ICM42607_RA_BLK_SEL_R                       0x7C
#define ICM42607_RA_MADDR_R                         0x7D
#define ICM42607_RA_M_R                             0x7E

/* ===== MREG1 register addresses (accessed via BLK_SEL/MADDR) ===== */
#define ICM42607_MREG1_SENSOR_CONFIG3               0x06

/* ===== SIGNAL_PATH_RESET bits ===== */
#define ICM42607_SOFT_RESET_DEVICE_CONFIG           (1 << 4)

/* ===== INT_STATUS (0x3A) bits ===== */
#define ICM42607_INT_STATUS_RESET_DONE              (1 << 4)

/* ===== PWR_MGMT0 bits ===== */
#define ICM42607_PWR_MGMT0_GYRO_MODE_LN             (3 << 2)
#define ICM42607_PWR_MGMT0_ACCEL_MODE_LN            (3 << 0)

/* ===== INT_CONFIG bits ===== */
#define ICM42607_INT1_MODE_PULSED                   (0 << 2)
#define ICM42607_INT1_MODE_LATCHED                  (1 << 2)
#define ICM42607_INT1_DRIVE_CIRCUIT_PP              (1 << 1)
#define ICM42607_INT1_POLARITY_ACTIVE_HIGH          (1 << 0)
#define ICM42607_INT1_POLARITY_ACTIVE_LOW           (0 << 0)

/* ===== INT_SOURCE0 bits ===== */
#define ICM42607_DRDY_INT1_EN                       (1 << 3)

/* ===== GYRO_CONFIG0 ===== */
/* bits [6:5] GYRO_FS_SEL, bits [3:0] GYRO_ODR */
#define ICM42607_GYRO_FS_2000DPS                    (0x00 << 5)
#define ICM42607_GYRO_FS_1000DPS                    (0x01 << 5)
#define ICM42607_GYRO_FS_500DPS                     (0x02 << 5)
#define ICM42607_GYRO_FS_250DPS                     (0x03 << 5)

/* ===== ACCEL_CONFIG0 ===== */
/* bits [6:5] ACCEL_UI_FS_SEL, bits [3:0] ACCEL_ODR */
#define ICM42607_ACCEL_FS_16G                       (0x00 << 5)
#define ICM42607_ACCEL_FS_8G                        (0x01 << 5)
#define ICM42607_ACCEL_FS_4G                        (0x02 << 5)
#define ICM42607_ACCEL_FS_2G                        (0x03 << 5)

/* ===== ODR values (shared for gyro and accel, bits [3:0]) ===== */
#define ICM42607_ODR_1600HZ                         0x05
#define ICM42607_ODR_800HZ                          0x06
#define ICM42607_ODR_400HZ                          0x07
#define ICM42607_ODR_200HZ                          0x08
#define ICM42607_ODR_100HZ                          0x09

/* ===== GYRO_CONFIG1 ===== */
/* bits [2:0] GYRO_UI_FILT_BW */
#define ICM42607_GYRO_FILT_BW_BYPASSED              0x00
#define ICM42607_GYRO_FILT_BW_180HZ                 0x01
#define ICM42607_GYRO_FILT_BW_121HZ                 0x02
#define ICM42607_GYRO_FILT_BW_73HZ                  0x03
#define ICM42607_GYRO_FILT_BW_53HZ                  0x04
#define ICM42607_GYRO_FILT_BW_34HZ                  0x05
#define ICM42607_GYRO_FILT_BW_25HZ                  0x06
#define ICM42607_GYRO_FILT_BW_16HZ                  0x07

/* ===== ACCEL_CONFIG1 ===== */
/* bits [2:0] ACCEL_UI_FILT_BW */
#define ICM42607_ACCEL_FILT_BW_BYPASSED             0x00
#define ICM42607_ACCEL_FILT_BW_180HZ                0x01
#define ICM42607_ACCEL_FILT_BW_121HZ                0x02
#define ICM42607_ACCEL_FILT_BW_73HZ                 0x03
#define ICM42607_ACCEL_FILT_BW_53HZ                 0x04
#define ICM42607_ACCEL_FILT_BW_34HZ                 0x05
#define ICM42607_ACCEL_FILT_BW_25HZ                 0x06
#define ICM42607_ACCEL_FILT_BW_16HZ                 0x07

/* ===== INT_STATUS_DRDY (Bank 0, 0x39) ===== */
#define ICM42607_RA_INT_STATUS_DRDY                 0x39
#define ICM42607_INT_STATUS_DRDY_DATA_RDY           (1 << 0)

/* ===== INTF_CONFIG0 bits ===== */
#define ICM42607_SENSOR_DATA_ENDIAN_BIG             (1 << 4)
#define ICM42607_FIFO_COUNT_ENDIAN_BIG              (1 << 5)

/* ===== INTF_CONFIG1 AFSR (Anti-aliased Filter Spectral Response) bits ===== */
#define ICM42607_INTF_CONFIG1_AFSR_MASK             0xC0
#define ICM42607_INTF_CONFIG1_AFSR_DISABLE          0x40

/* ===== MCLK_RDY bit ===== */
#define ICM42607_MCLK_RDY_BIT                       (1 << 3)

/* ICM42607-P (0x60) and ICM42670-P (0x67) share register map. */
static bool icm42607WhoAmIValid(uint8_t whoami)
{
    return whoami == ICM42607_WHO_AM_I_CONST || whoami == ICM42670P_WHO_AM_I_CONST;
}

static bool icm42607WaitMclkReady(const busDevice_t *dev)
{
    for (uint8_t i = 0; i < 10; i++) {
        uint8_t mclkRdy;
        busRead(dev, ICM42607_RA_MCLK_RDY, &mclkRdy);
        if ((mclkRdy & ICM42607_MCLK_RDY_BIT) != 0) {
            return true;
        }
        delayMicroseconds(100);
    }
    return false;
}

static void icm42607WriteMREG(const busDevice_t *dev, uint8_t bank, uint8_t reg, uint8_t val)
{
    icm42607WaitMclkReady(dev);
    busWrite(dev, ICM42607_RA_BLK_SEL_W, bank);
    busWrite(dev, ICM42607_RA_MADDR_W, reg);
    busWrite(dev, ICM42607_RA_M_W, val);
    delayMicroseconds(10);
    busWrite(dev, ICM42607_RA_BLK_SEL_W, 0x00);
}

static const gyroFilterAndRateConfig_t icm42607GyroConfigs[] = {
    /*                            DLPF            ODR  */
    { GYRO_LPF_256HZ,   1600,   { 1,    ICM42607_ODR_1600HZ } },
    { GYRO_LPF_256HZ,    800,   { 1,    ICM42607_ODR_800HZ  } },
    { GYRO_LPF_256HZ,    400,   { 2,    ICM42607_ODR_400HZ  } },
    { GYRO_LPF_188HZ,    400,   { 1,    ICM42607_ODR_400HZ  } },
    { GYRO_LPF_98HZ,     200,   { 2,    ICM42607_ODR_200HZ  } },
    { GYRO_LPF_42HZ,     200,   { 4,    ICM42607_ODR_200HZ  } },
    { GYRO_LPF_20HZ,     100,   { 5,    ICM42607_ODR_100HZ  } },
    { GYRO_LPF_10HZ,     100,   { 6,    ICM42607_ODR_100HZ  } },
};

static void icm42607AccInit(accDev_t *acc)
{
    acc->acc_1G = 512 * 4;   // 16g => 2048 LSB/g
}

static bool icm42607AccRead(accDev_t *acc)
{
    uint8_t data[6];

    const bool ack = busReadBuf(acc->busDev, ICM42607_RA_ACCEL_DATA_X1, data, 6);
    if (!ack) {
        return false;
    }

    acc->ADCRaw[X] = (float) int16_val_big_endian(data, 0);
    acc->ADCRaw[Y] = (float) int16_val_big_endian(data, 1);
    acc->ADCRaw[Z] = (float) int16_val_big_endian(data, 2);

    return true;
}

bool icm42607AccDetect(accDev_t *acc)
{
    uint8_t whoami;
    bool usedSlowBusFallback = false;

    acc->busDev = busDeviceOpen(BUSTYPE_ANY, DEVHW_ICM42607, acc->imuSensorToUse);
    if (acc->busDev == NULL) {
        return false;
    }

    mpuContextData_t *ctx = busDeviceGetScratchpadMemory(acc->busDev);
    if (ctx->chipMagicNumber != 0x4207) {
        // Fallback for cases where scratchpad magic is stale/cleared but gyro was detected.
        busSetSpeed(acc->busDev, BUS_SPEED_INITIALIZATION);
        usedSlowBusFallback = true;
        busRead(acc->busDev, ICM42607_RA_WHO_AM_I, &whoami);
        if (!icm42607WhoAmIValid(whoami)) {
            if (usedSlowBusFallback) {
                busSetSpeed(acc->busDev, BUS_SPEED_FAST);
            }
            return false;
        }

        // Refresh magic to avoid repeatedly taking the fallback path.
        ctx->chipMagicNumber = 0x4207;
    }

    // Ensure runtime SPI is restored after low-speed identity probe.
    if (usedSlowBusFallback) {
        busSetSpeed(acc->busDev, BUS_SPEED_FAST);
    }

    acc->initFn = icm42607AccInit;
    acc->readFn = icm42607AccRead;
    acc->accAlign = acc->busDev->param;

    return true;
}

static void icm42607AccAndGyroInit(gyroDev_t *gyro)
{
    busDevice_t *dev = gyro->busDev;
    const gyroFilterAndRateConfig_t *config = chooseGyroConfig(gyro->lpf, 1000000 / gyro->requestedSampleIntervalUs,
                                                               &icm42607GyroConfigs[0], ARRAYLEN(icm42607GyroConfigs));
    gyro->sampleRateIntervalUs = 1000000 / config->gyroRateHz;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    /* ---- Reset ---- */
    busWrite(dev, ICM42607_RA_SIGNAL_PATH_RESET, ICM42607_SOFT_RESET_DEVICE_CONFIG);
    delay(2);

    for (uint8_t tries = 0; tries < 50; tries++) {
        uint8_t whoami;
        uint8_t sigPathReset;
        busRead(dev, ICM42607_RA_WHO_AM_I, &whoami);
        busRead(dev, ICM42607_RA_SIGNAL_PATH_RESET, &sigPathReset);

        if (icm42607WhoAmIValid(whoami) &&
            (sigPathReset & ICM42607_SOFT_RESET_DEVICE_CONFIG) == 0) {
            break;
        }
        delay(10);
    }

    /* ---- Wait for MCLK ---- */
    for (uint8_t tries = 0; tries < 50; tries++) {
        uint8_t mclkRdy;
        busRead(dev, ICM42607_RA_MCLK_RDY, &mclkRdy);
        if ((mclkRdy & ICM42607_MCLK_RDY_BIT) != 0) {
            break;
        }
        delay(5);
    }

    /* ---- Set sensor data endianness (read-modify-write to preserve other bits) ---- */
    uint8_t intfConfig0;
    busRead(dev, ICM42607_RA_INTF_CONFIG0, &intfConfig0);
    intfConfig0 |= ICM42607_FIFO_COUNT_ENDIAN_BIG | ICM42607_SENSOR_DATA_ENDIAN_BIG;
    busWrite(dev, ICM42607_RA_INTF_CONFIG0, intfConfig0);
    delay(1);

    /* ---- Power on gyro and accel in Low Noise mode ---- */
    busWrite(dev, ICM42607_RA_PWR_MGMT0, ICM42607_PWR_MGMT0_GYRO_MODE_LN | ICM42607_PWR_MGMT0_ACCEL_MODE_LN);
    delay(50);

    /* ---- Disable APEX features via MREG1 SENSOR_CONFIG3 ---- */
    icm42607WriteMREG(dev, 0x00, ICM42607_MREG1_SENSOR_CONFIG3, 0x40);

    /* ---- Configure gyro — ±2000 dps, selected ODR ---- */
    busWrite(dev, ICM42607_RA_GYRO_CONFIG0, ICM42607_GYRO_FS_2000DPS | (config->gyroConfigValues[1] & 0x0F));
    delay(15);

    /* ---- Configure accel — ±16g, same ODR as gyro ---- */
    busWrite(dev, ICM42607_RA_ACCEL_CONFIG0, ICM42607_ACCEL_FS_16G | (config->gyroConfigValues[1] & 0x0F));
    delay(15);

    /* ---- Configure gyro LPF (read-modify-write to preserve reserved bits) ---- */
    uint8_t gyroConfig1;
    busRead(dev, ICM42607_RA_GYRO_CONFIG1, &gyroConfig1);
    gyroConfig1 = (gyroConfig1 & 0xF8) | (config->gyroConfigValues[0] & 0x07);
    busWrite(dev, ICM42607_RA_GYRO_CONFIG1, gyroConfig1);
    delay(15);

    /* ---- Configure accel LPF ---- */
    uint8_t accelConfig1;
    busRead(dev, ICM42607_RA_ACCEL_CONFIG1, &accelConfig1);
    accelConfig1 = (accelConfig1 & 0xF8) | ICM42607_ACCEL_FILT_BW_53HZ;
    busWrite(dev, ICM42607_RA_ACCEL_CONFIG1, accelConfig1);
    delay(15);

    /* ---- Configure INT1 — pulsed, push-pull, active high ---- */
    busWrite(dev, ICM42607_RA_INT_CONFIG, ICM42607_INT1_MODE_PULSED | ICM42607_INT1_DRIVE_CIRCUIT_PP | ICM42607_INT1_POLARITY_ACTIVE_HIGH);
    delay(15);

    /* ---- Enable data ready interrupt on INT1 ---- */
    busWrite(dev, ICM42607_RA_INT_SOURCE0, ICM42607_DRDY_INT1_EN);
    delay(100);

    /* Attempt AFSR disable via INTF_CONFIG1 bits [7:6].
     * On ICM42605/42688P these bits control AFSR at 0x4D.
     * On ICM42607 the register is at 0x36 and bits [7:4] are marked "reserved",
     * but reserved bits may still control AFSR (undocumented). */
    uint8_t intfConfig1;
    busRead(dev, ICM42607_RA_INTF_CONFIG1, &intfConfig1);
    intfConfig1 &= ~ICM42607_INTF_CONFIG1_AFSR_MASK;
    intfConfig1 |= ICM42607_INTF_CONFIG1_AFSR_DISABLE;
    busWrite(dev, ICM42607_RA_INTF_CONFIG1, intfConfig1);
    delay(15);

    busSetSpeed(dev, BUS_SPEED_FAST);
}

static bool icm42607DeviceDetect(busDevice_t *dev)
{
    uint8_t mclk;
    uint8_t whoami;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    for (uint8_t attempt = 0; attempt < 5; attempt++) {
        busWrite(dev, ICM42607_RA_SIGNAL_PATH_RESET, ICM42607_SOFT_RESET_DEVICE_CONFIG);
        delay(2);

        busWrite(dev, ICM42607_RA_PWR_MGMT0,
                 ICM42607_PWR_MGMT0_GYRO_MODE_LN | ICM42607_PWR_MGMT0_ACCEL_MODE_LN);
        delay(1);

        for (uint8_t i = 0; i < 20; i++) {
            busRead(dev, ICM42607_RA_MCLK_RDY, &mclk);
            if (mclk & ICM42607_MCLK_RDY_BIT) {
                break;
            }
            delay(1);
        }

        busRead(dev, ICM42607_RA_WHO_AM_I, &whoami);
        if (icm42607WhoAmIValid(whoami)) {
            return true;
        }

        delay(10);
    }

    return false;
}

static bool icm42607GyroRead(gyroDev_t *gyro)
{
    uint8_t data[6];

    const bool ack = busReadBuf(gyro->busDev, ICM42607_RA_GYRO_DATA_X1, data, 6);
    if (!ack) {
        return false;
    }

    gyro->gyroADCRaw[X] = (float) int16_val_big_endian(data, 0);
    gyro->gyroADCRaw[Y] = (float) int16_val_big_endian(data, 1);
    gyro->gyroADCRaw[Z] = (float) int16_val_big_endian(data, 2);

    return true;
}

static bool icm42607ReadTemperature(gyroDev_t *gyro, int16_t *temp)
{
    uint8_t data[2];

    const bool ack = busReadBuf(gyro->busDev, ICM42607_RA_TEMP_DATA1, data, 2);
    if (!ack) {
        return false;
    }
    // Temperature in Degrees Centigrade = (TEMP_DATA / 128) + 25
    // Store as degC * 10
    *temp = (int16_val_big_endian(data, 0) / 12.8f) + 250;

    return true;
}

bool icm42607GyroDetect(gyroDev_t *gyro)
{
    gyro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_ICM42607, gyro->imuSensorToUse, OWNER_MPU);
    if (gyro->busDev == NULL) {
        return false;
    }

    if (!icm42607DeviceDetect(gyro->busDev)) {
        busDeviceDeInit(gyro->busDev);
        return false;
    }

    // Magic number for ACC detection
    mpuContextData_t *ctx = busDeviceGetScratchpadMemory(gyro->busDev);
    ctx->chipMagicNumber = 0x4207;

    gyro->initFn = icm42607AccAndGyroInit;
    gyro->readFn = icm42607GyroRead;
    gyro->intStatusFn = gyroCheckDataReady;
    gyro->temperatureFn = icm42607ReadTemperature;
    gyro->scale = 1.0f / 16.4f;     // 2000 dps => 16.4 LSB/dps
    gyro->gyroAlign = gyro->busDev->param;

    return true;
}

#endif
