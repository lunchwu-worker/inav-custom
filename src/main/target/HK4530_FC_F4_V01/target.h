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

#pragma once

#define USE_TARGET_CONFIG

#define TARGET_BOARD_IDENTIFIER "H4F4"
#define USBD_PRODUCT_STRING     "HK4530-FC-F4-V01"

// ******** LEDs **********************
#define LED0                    PC5  // Green

// ******** Beeper ********************
#define BEEPER                  PA4
#define BEEPER_INVERTED

/*
 * Buses
 */

// SPI2 (IMU)
#define USE_SPI
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

// SPI3 (Flash)
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5

/*
 * Serial ports
 */
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PB6
#define UART1_RX_PIN            PB7

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

#define USE_UART3
#define UART3_TX_PIN            PC10
#define UART3_RX_PIN            PC11

#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1

#define USE_UART5
#define UART5_TX_PIN            PC12
#define UART5_RX_PIN            PD2

#define USE_UART6               // VIDEO (analog VTX)
#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7

// Optional Softserial on UART2 TX Pin PA2
#define USE_SOFTSERIAL1
#define SOFTSERIAL_1_TX_PIN     PA2
#define SOFTSERIAL_1_RX_PIN     PA2

#define SERIAL_PORT_COUNT       8

/*
 * I2C
 */
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

/*
 * Sensor drivers
 */

// ICM42605/ICM42688P
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW0_DEG
#define ICM42605_CS_PIN         PA5
#define ICM42605_SPI_BUS        BUS_SPI2
#define ICM42605_EXTI_PIN       PB12

// LSM6DSV16X (optional, shares same SPI bus and CS pin)
#define USE_IMU_LSM6DSV16X
#define IMU_LSM6DSV16X_ALIGN    CW0_DEG
#define LSM6DSV16X_CS_PIN       PA5
#define LSM6DSV16X_SPI_BUS      BUS_SPI2

// *************** Baro **************************
#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_BMP085
#define USE_BARO_DPS310
#define USE_BARO_SPL06

// *************** Mag ***************************
#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

#define RANGEFINDER_I2C_BUS     BUS_I2C1
#define PITOT_I2C_BUS           BUS_I2C1
#define TEMPERATURE_I2C_BUS     BUS_I2C1

// *************** Flash *************************
// W25Q128 / PY25Q128HA on SPI3
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_CS_PIN           PA15
#define M25P16_SPI_BUS          BUS_SPI3

#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** RX ****************************
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART2

// *************** ADC ***************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_1
#define VBAT_ADC_CHANNEL            ADC_CHN_2

// *************** LED Strip *********************
#define USE_LED_STRIP
#define WS2811_PIN              PA7

// *************** OTHERS ************************
#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_BLACKBOX | FEATURE_AIRMODE)
#define CURRENT_METER_SCALE     250

#define USE_DSHOT
#define USE_DSHOT_DMAR
#define USE_DSHOT_BITBANG
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define MAX_PWM_OUTPUT_PORTS    10

// *************** PINIO *************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN              PC4     // BT (Bluetooth)
#define PINIO2_PIN              PC13    // CAM Switch
#define PINIO3_PIN              PC14    // 10V power output enable (high = on)
