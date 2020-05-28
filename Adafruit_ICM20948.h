/*!
 *  @file Adafruit_ICM20948.h
 *
 * 	I2C Driver for the Adafruit ICM20948 9-DoF Accelerometer, Gyro, and
 *Magnetometer library
 *
 * 	This is a library for the Adafruit ICM20948 breakout:
 * 	https://www.adafruit.com/products/4554
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *
 *	BSD license (see license.txt)
 */

#ifndef _ADAFRUIT_ICM20948_H
#define _ADAFRUIT_ICM20948_H

#include "Adafruit_ICM20X.h"

#define I2C_MASTER_RESETS_BEFORE_FAIL                                          \
  5 ///< The number of times to try resetting a stuck I2C master before giving
    ///< up
#define NUM_FINISHED_CHECKS                                                    \
  100 ///< How many times to poll I2C_SLV4_DONE before giving up and resetting
#define ICM20948_I2CADDR_DEFAULT 0x69 ///< ICM20948 default i2c address
#define ICM20948_MAG_ID 0x09          ///< The chip ID for the magnetometer
// Bank 0
#define ICM20948_I2C_MST_STATUS                                                \
  0x17 ///< Records if I2C master bus data is finished
// Bank 3
#define ICM20948_I2C_MST_ODR_CONFIG 0x0 ///< Sets ODR for I2C master bus
#define ICM20948_I2C_MST_CTRL 0x1       ///< I2C master bus config
#define ICM20948_I2C_MST_DELAY_CTRL 0x2 ///< I2C master bus config
#define ICM20948_I2C_SLV0_ADDR                                                 \
  0x3 ///< Sets I2C address for I2C master bus slave 0
#define ICM20948_I2C_SLV0_REG                                                  \
  0x4 ///< Sets register address for I2C master bus slave 0
#define ICM20948_I2C_SLV0_CTRL 0x5 ///< Controls for I2C master bus slave 0
#define ICM20948_I2C_SLV0_DO 0x6   ///< Sets I2C master bus slave 0 data out

#define ICM20948_I2C_SLV4_ADDR                                                 \
  0x13 ///< Sets I2C address for I2C master bus slave 4
#define ICM20948_I2C_SLV4_REG                                                  \
  0x14 ///< Sets register address for I2C master bus slave 4
#define ICM20948_I2C_SLV4_CTRL 0x15 ///< Controls for I2C master bus slave 4
#define ICM20948_I2C_SLV4_DO 0x16   ///< Sets I2C master bus slave 4 data out
#define ICM20948_I2C_SLV4_DI 0x17   ///< Sets I2C master bus slave 4 data in

#define ICM20948_UT_PER_LSB 0.15 ///< mag data LSB value (fixed)

#define AK09916_WIA2 0x01  ///< Magnetometer
#define AK09916_ST1 0x10   ///< Magnetometer
#define AK09916_HXL 0x11   ///< Magnetometer
#define AK09916_HXH 0x12   ///< Magnetometer
#define AK09916_HYL 0x13   ///< Magnetometer
#define AK09916_HYH 0x14   ///< Magnetometer
#define AK09916_HZL 0x15   ///< Magnetometer
#define AK09916_HZH 0x16   ///< Magnetometer
#define AK09916_ST2 0x18   ///< Magnetometer
#define AK09916_CNTL2 0x31 ///< Magnetometer
#define AK09916_CNTL3 0x32 ///< Magnetometer

/** The accelerometer data range */
typedef enum {
  ICM20948_ACCEL_RANGE_2_G,
  ICM20948_ACCEL_RANGE_4_G,
  ICM20948_ACCEL_RANGE_8_G,
  ICM20948_ACCEL_RANGE_16_G,
} icm20948_accel_range_t;

/** The gyro data range */
typedef enum {
  ICM20948_GYRO_RANGE_250_DPS,
  ICM20948_GYRO_RANGE_500_DPS,
  ICM20948_GYRO_RANGE_1000_DPS,
  ICM20948_GYRO_RANGE_2000_DPS,
} icm20948_gyro_range_t;

/*!
 *    @brief  Class that stores state and functions for interacting with
 *            the ST ICM2948 9-DoF Accelerometer, gyro, and magnetometer
 */
class Adafruit_ICM20948 : public Adafruit_ICM20X {
public:
  Adafruit_ICM20948();
  ~Adafruit_ICM20948(){};
  bool begin_I2C(uint8_t i2c_addr = ICM20948_I2CADDR_DEFAULT,
                 TwoWire *wire = &Wire, int32_t sensor_id = 0);

  icm20948_accel_range_t getAccelRange(void);
  void setAccelRange(icm20948_accel_range_t new_accel_range);

  icm20948_gyro_range_t getGyroRange(void);
  void setGyroRange(icm20948_gyro_range_t new_gyro_range);

private:
  uint8_t _read_ext_reg(uint8_t slv_addr, uint8_t reg_addr);
  bool _write_ext_reg(uint8_t slv_addr, uint8_t reg_addr, uint8_t value);

  uint8_t _read_mag_reg(uint8_t reg_addr);
  bool _write_mag_reg(uint8_t reg_addr, uint8_t value);

  uint8_t getMagId(void);

  bool _setupMag(void);
  void _scale_values(void);
  void _resetI2CMaster(void);
};

#endif