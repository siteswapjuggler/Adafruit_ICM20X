
/*!
 *  @file Adafruit_ICM20X.cpp
 *
 *  @mainpage Adafruit ICM20X family motion sensor library
 *
 *  @section intro_sec Introduction
 *
 * 	I2C Driver for the Adafruit ICM20X Family of motion sensors
 *
 * 	This is a library for the Adafruit ICM20X breakouts:
 * 	https://www.adafruit.com/product/4464
 * 	https://www.adafruit.com/product/4554
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *  @section dependencies Dependencies
 *  This library depends on the Adafruit BusIO library
 *
 *  This library depends on the Adafruit Unified Sensor library
 *
 *  @section author Author
 *
 *  Bryan Siepert for Adafruit Industries
 *
 * 	@section license License
 *
 * 	BSD (see license.txt)
 *
 * 	@section  HISTORY
 *
 *     v1.0 - First release
 */

#include "Arduino.h"
#include <Wire.h>

#include "Adafruit_ICM20X.h"

/*!
 *    @brief  Instantiates a new ICM20X class!
 */
Adafruit_ICM20X::Adafruit_ICM20X(void) {}

/*!
 *    @brief  Cleans up the ICM20X
 */
Adafruit_ICM20X::~Adafruit_ICM20X(void) {
  if (accel_sensor)
    delete accel_sensor;
  if (gyro_sensor)
    delete gyro_sensor;
  if (mag_sensor)
    delete mag_sensor;
  if (temp_sensor)
    delete temp_sensor;
}

/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  i2c_address
 *            The I2C address to be used.
 *    @param  wire
 *            The Wire object to be used for I2C connections.
 *    @param  sensor_id
 *            An optional parameter to set the sensor ids to differentiate
 * similar sensors The passed value is assigned to the accelerometer, the gyro
 * gets +1, the magnetometer +2, and the temperature sensor +3.
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_ICM20X::begin_I2C(uint8_t i2c_address, TwoWire *wire,
                                int32_t sensor_id) {
  return false;
}

/*!
 *    @brief  Sets up the hardware and initializes hardware SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  theSPI The SPI object to be used for SPI connections.
 *    @param  sensor_id An optional parameter to set the sensor ids to
 * differentiate similar sensors The passed value is assigned to the
 * accelerometer, the gyro gets +1, the magnetometer +2, and the temperature
 * sensor +3.
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_ICM20X::begin_SPI(uint8_t cs_pin, SPIClass *theSPI,
                                int32_t sensor_id) {
  i2c_dev = NULL;

  if (spi_dev) {
    delete spi_dev; // remove old interface
  }

  spi_dev = new Adafruit_SPIDevice(cs_pin,
                                   1000000,               // frequency
                                   SPI_BITORDER_MSBFIRST, // bit order
                                   SPI_MODE0,             // data mode
                                   theSPI);

  if (!spi_dev->begin()) {
    return false;
  }

  return _init(sensor_id);
}

/*!
 *    @brief  Sets up the hardware and initializes software SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  sck_pin The arduino pin # connected to SPI clock
 *    @param  miso_pin The arduino pin # connected to SPI MISO
 *    @param  mosi_pin The arduino pin # connected to SPI MOSI
 *    @param  sensor_id An optional parameter to set the sensor ids to
 * differentiate similar sensors The passed value is assigned to the
 * accelerometer, the gyro gets +1, the magnetometer +2, and the temperature
 * sensor +3.
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_ICM20X::begin_SPI(int8_t cs_pin, int8_t sck_pin, int8_t miso_pin,
                                int8_t mosi_pin, int32_t sensor_id) {
  i2c_dev = NULL;

  if (spi_dev) {
    delete spi_dev; // remove old interface
  }
  spi_dev = new Adafruit_SPIDevice(cs_pin, sck_pin, miso_pin, mosi_pin,
                                   1000000,               // frequency
                                   SPI_BITORDER_MSBFIRST, // bit order
                                   SPI_MODE0);            // data mode
  if (!spi_dev->begin()) {
    return false;
  }

  return _init(sensor_id);
}

/**
 * @brief Reset the internal registers and restores the default settings
 *
 */
void Adafruit_ICM20X::reset(void) {
  _setBank(0);

  Adafruit_BusIO_Register pwr_mgmt1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_PWR_MGMT_1, 1);

  Adafruit_BusIO_RegisterBits reset_bit =
      Adafruit_BusIO_RegisterBits(&pwr_mgmt1, 1, 7);

  reset_bit.write(1);
  delay(20);

  while (reset_bit.read()) {
    delay(10);
  };
  delay(50);
}

/*!  @brief Initilizes the sensor
 *   @param sensor_id Optional unique ID for the sensor set
 *   @returns True if chip identified and initialized
 */
bool Adafruit_ICM20X::_init(int32_t sensor_id) {
  Adafruit_BusIO_Register chip_id = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_WHOAMI);

  _setBank(0);
  uint8_t chip_id_ = chip_id.read();

  if ((chip_id_ != ICM20649_CHIP_ID) && (chip_id_ != ICM20948_CHIP_ID)) {
    return false;
  }

  _sensorid_accel = sensor_id;
  _sensorid_gyro = sensor_id + 1;
  _sensorid_mag = sensor_id + 2;
  _sensorid_temp = sensor_id + 3;

  reset();

  Adafruit_BusIO_Register pwr_mgmt_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_PWR_MGMT_1);
  Adafruit_BusIO_Register lp_config = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_LP_CONFIG);

  Adafruit_BusIO_RegisterBits sleep =
      Adafruit_BusIO_RegisterBits(&pwr_mgmt_1, 1, 6);

  Adafruit_BusIO_RegisterBits clock_source =
      Adafruit_BusIO_RegisterBits(&pwr_mgmt_1, 3, 0);

  Adafruit_BusIO_RegisterBits i2c_mst_cycle =
      Adafruit_BusIO_RegisterBits(&lp_config, 1, 6);

  Adafruit_BusIO_Register gyro_config_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_GYRO_CONFIG_1);

  Adafruit_BusIO_Register accel_config_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_ACCEL_CONFIG_1);

  Adafruit_BusIO_RegisterBits gyro_filter_bit =
      Adafruit_BusIO_RegisterBits(&gyro_config_1, 1, 0);
  Adafruit_BusIO_RegisterBits accel_filter_bit =
      Adafruit_BusIO_RegisterBits(&accel_config_1, 1, 0);

  sleep.write(false); // take out of default sleep state
  // AUTO SELECT BEST CLOCK, required by DS to meet specs
  clock_source.write(1);

  i2c_mst_cycle.write(1);

  init1();

  // writeGyroRange(3);
  // writeAccelRange(3);
  // // 1100Hz/(1+10) = 100Hz
  // setGyroRateDivisor(10);

  // // # 1125Hz/(1+20) = 53.57Hz
  // setAccelRateDivisor(20);

  temp_sensor = new Adafruit_ICM20X_Temp(this);
  accel_sensor = new Adafruit_ICM20X_Accelerometer(this);
  gyro_sensor = new Adafruit_ICM20X_Gyro(this);
  mag_sensor = new Adafruit_ICM20X_Magnetometer(this);
  delay(20);

  return true;
}

void Adafruit_ICM20X::init1(void) {

  Adafruit_BusIO_Register gyro_config_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_GYRO_CONFIG_1);

  Adafruit_BusIO_Register accel_config_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_ACCEL_CONFIG_1);

  Adafruit_BusIO_RegisterBits accel_filter_bit =
      Adafruit_BusIO_RegisterBits(&accel_config_1, 1, 0);

  Adafruit_BusIO_RegisterBits accel_filter_cnf =
      Adafruit_BusIO_RegisterBits(&accel_config_1, 3, 3);

  Adafruit_BusIO_RegisterBits gyro_filter_bit =
      Adafruit_BusIO_RegisterBits(&gyro_config_1, 1, 0);

  Adafruit_BusIO_RegisterBits gyro_filter_cnf =
      Adafruit_BusIO_RegisterBits(&gyro_config_1, 3, 3);

  _setBank(2);

  accel_filter_bit.write(1);
  gyro_filter_bit.write(1);
  accel_filter_cnf.write(7);
  gyro_filter_cnf.write(7);
  accel_filter_bit.write(0);
  gyro_filter_bit.write(0);
}
/**************************************************************************/
/*!
    @brief  Gets the most recent sensor event, Adafruit Unified Sensor format
    @param  accel
            Pointer to an Adafruit Unified sensor_event_t object to be filled
            with acceleration event data.

    @param  gyro
            Pointer to an Adafruit Unified sensor_event_t object to be filled
            with gyro event data.

    @param  mag
            Pointer to an Adafruit Unified sensor_event_t object to be filled
            with magnetometer event data.

    @param  temp
            Pointer to an Adafruit Unified sensor_event_t object to be filled
            with temperature event data.

    @return True on successful read
*/
/**************************************************************************/
bool Adafruit_ICM20X::getEvent(sensors_event_t *accel, sensors_event_t *gyro,
                               sensors_event_t *temp, sensors_event_t *mag) {
  uint32_t t = millis();
  _read();

  // use helpers to fill in the events
  fillAccelEvent(accel, t);
  fillGyroEvent(gyro, t);
  fillTempEvent(temp, t);
  if (mag) {
    fillMagEvent(mag, t);
  }

  return true;
}

void Adafruit_ICM20X::fillAccelEvent(sensors_event_t *accel,
                                     uint32_t timestamp) {
  memset(accel, 0, sizeof(sensors_event_t));
  accel->version = 1;
  accel->sensor_id = _sensorid_accel;
  accel->type = SENSOR_TYPE_ACCELEROMETER;
  accel->timestamp = timestamp;

  accel->acceleration.x = accX * SENSORS_GRAVITY_EARTH;
  accel->acceleration.y = accY * SENSORS_GRAVITY_EARTH;
  accel->acceleration.z = accZ * SENSORS_GRAVITY_EARTH;
}

void Adafruit_ICM20X::fillGyroEvent(sensors_event_t *gyro, uint32_t timestamp) {
  memset(gyro, 0, sizeof(sensors_event_t));
  gyro->version = 1;
  gyro->sensor_id = _sensorid_gyro;
  gyro->type = SENSOR_TYPE_GYROSCOPE;
  gyro->timestamp = timestamp;
  gyro->gyro.x = gyroX * SENSORS_DPS_TO_RADS;
  gyro->gyro.y = gyroY * SENSORS_DPS_TO_RADS;
  gyro->gyro.z = gyroZ * SENSORS_DPS_TO_RADS;
}

void Adafruit_ICM20X::fillMagEvent(sensors_event_t *mag, uint32_t timestamp) {
  memset(mag, 0, sizeof(sensors_event_t));
  mag->version = 1;
  mag->sensor_id = _sensorid_mag;
  mag->type = SENSOR_TYPE_MAGNETIC_FIELD;
  mag->timestamp = timestamp;
  mag->magnetic.x = magX; // magic number!
  mag->magnetic.y = magY;
  mag->magnetic.z = magZ;
}

void Adafruit_ICM20X::fillTempEvent(sensors_event_t *temp, uint32_t timestamp) {

  memset(temp, 0, sizeof(sensors_event_t));
  temp->version = sizeof(sensors_event_t);
  temp->sensor_id = _sensorid_temp;
  temp->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  temp->timestamp = timestamp;
  temp->temperature = (temperature / 333.87) + 21.0;
}
/******************* Adafruit_Sensor functions *****************/
/*!
 *     @brief  Updates the measurement data for all sensors simultaneously
 */
/**************************************************************************/
void Adafruit_ICM20X::_read(void) {

  _setBank(0);

  // reading 9 bytes of mag data to fetch the register that tells the mag we've
  // read all the data
  const uint8_t numbytes = 14 + 9; // Read Accel, gyro, temp, and 9 bytes of mag

  Adafruit_BusIO_Register data_reg = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_ACCEL_XOUT_H, numbytes);

  uint8_t buffer[numbytes];
  data_reg.read(buffer, numbytes);

  rawAccX = buffer[0] << 8 | buffer[1];
  rawAccY = buffer[2] << 8 | buffer[3];
  rawAccZ = buffer[4] << 8 | buffer[5];

  rawGyroX = buffer[6] << 8 | buffer[7];
  rawGyroY = buffer[8] << 8 | buffer[9];
  rawGyroZ = buffer[10] << 8 | buffer[11];

  temperature = buffer[12] << 8 | buffer[13];

  rawMagX = ((buffer[16] << 8) |
             (buffer[15] & 0xFF)); // Mag data is read little endian
  rawMagY = ((buffer[18] << 8) | (buffer[17] & 0xFF));
  rawMagZ = ((buffer[20] << 8) | (buffer[19] & 0xFF));

  _scale_values();
  _setBank(0);
}
/**
 * @brief Scales the raw variables based on the current measurement range
 *
 */
void Adafruit_ICM20X::_scale_values(void) {}

/*!
    @brief  Gets an Adafruit Unified Sensor object for the accelerometer
    sensor component
    @return Adafruit_Sensor pointer to accelerometer sensor
 */
Adafruit_Sensor *Adafruit_ICM20X::getAccelerometerSensor(void) {
  return accel_sensor;
}

/*!
    @brief  Gets an Adafruit Unified Sensor object for the gyro sensor component
    @return Adafruit_Sensor pointer to gyro sensor
 */
Adafruit_Sensor *Adafruit_ICM20X::getGyroSensor(void) { return gyro_sensor; }

/*!
    @brief  Gets an Adafruit Unified Sensor object for the magnetometer sensor
   component
    @return Adafruit_Sensor pointer to magnetometer sensor
 */
Adafruit_Sensor *Adafruit_ICM20X::getMagnetometerSensor(void) {
  return mag_sensor;
}

/*!
    @brief  Gets an Adafruit Unified Sensor object for the temp sensor component
    @return Adafruit_Sensor pointer to temperature sensor
 */
Adafruit_Sensor *Adafruit_ICM20X::getTemperatureSensor(void) {
  return temp_sensor;
}
/**************************************************************************/
/*!
    @brief Sets register bank.
    @param  bank_number
          The bank to set to active
*/
void Adafruit_ICM20X::_setBank(uint8_t bank_number) {

  Adafruit_BusIO_Register reg_bank_sel = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_REG_BANK_SEL);

  reg_bank_sel.write((bank_number & 0b11) << 4);
}

/**************************************************************************/
/*!
    @brief Get the accelerometer's measurement range.
    @returns The accelerometer's measurement range (`icm20x_accel_range_t`).
*/
uint8_t Adafruit_ICM20X::readAccelRange(void) {
  _setBank(2);

  Adafruit_BusIO_Register accel_config_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_ACCEL_CONFIG_1);

  Adafruit_BusIO_RegisterBits accel_range =
      Adafruit_BusIO_RegisterBits(&accel_config_1, 2, 1);

  uint8_t range = accel_range.read();
  _setBank(0);
  return range;
}

/**************************************************************************/
/*!

    @brief Sets the accelerometer's measurement range.
    @param  new_accel_range
            Measurement range to be set. Must be an
            `icm20x_accel_range_t`.
*/
void Adafruit_ICM20X::writeAccelRange(uint8_t new_accel_range) {
  _setBank(2);

  Adafruit_BusIO_Register accel_config_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_ACCEL_CONFIG_1);

  Adafruit_BusIO_RegisterBits accel_range =
      Adafruit_BusIO_RegisterBits(&accel_config_1, 2, 1);

  accel_range.write(new_accel_range);
  _setBank(0);
}

/**************************************************************************/
/*!
    @brief Get the gyro's measurement range.
    @returns The gyro's measurement range (`icm20x_gyro_range_t`).
*/
uint8_t Adafruit_ICM20X::readGyroRange(void) {
  _setBank(2);

  Adafruit_BusIO_Register gyro_config_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_GYRO_CONFIG_1);

  Adafruit_BusIO_RegisterBits gyro_range =
      Adafruit_BusIO_RegisterBits(&gyro_config_1, 2, 1);

  uint8_t range = gyro_range.read();
  _setBank(0);
  return range;
}

/**************************************************************************/
/*!

    @brief Sets the gyro's measurement range.
    @param  new_gyro_range
            Measurement range to be set. Must be an
            `icm20x_gyro_range_t`.
*/
void Adafruit_ICM20X::writeGyroRange(uint8_t new_gyro_range) {
  _setBank(2);

  Adafruit_BusIO_Register gyro_config_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_GYRO_CONFIG_1);

  Adafruit_BusIO_RegisterBits gyro_range =
      Adafruit_BusIO_RegisterBits(&gyro_config_1, 2, 1);

  gyro_range.write(new_gyro_range);
  _setBank(0);
}

/**************************************************************************/
/*!
    @brief Get the accelerometer's data rate divisor.
    @returns The accelerometer's data rate divisor (`uint8_t`).
*/
uint16_t Adafruit_ICM20X::getAccelRateDivisor(void) {
  _setBank(2);

  Adafruit_BusIO_Register accel_rate_divisor =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD,
                              ICM20X_ACCEL_SMPLRT_DIV_1, 2, MSBFIRST);

  uint16_t divisor_val = accel_rate_divisor.read();

  _setBank(0);
  return divisor_val;
}

/**************************************************************************/
/*!

    @brief Sets the accelerometer's data rate divisor.
    @param  new_accel_divisor
            The accelerometer's data rate divisor (`uint16_t`). This 12-bit
   value must be <= 4095
*/
void Adafruit_ICM20X::setAccelRateDivisor(uint16_t new_accel_divisor) {
  _setBank(2);

  Adafruit_BusIO_Register accel_rate_divisor =
      Adafruit_BusIO_Register(i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD,
                              ICM20X_ACCEL_SMPLRT_DIV_1, 2, MSBFIRST);

  accel_rate_divisor.write(new_accel_divisor);
  _setBank(0);
}

/**************************************************************************/
/*!
    @brief Get the gyro's data rate divisor.
    @returns The gyro's data rate divisor (`uint8_t`).
*/
uint8_t Adafruit_ICM20X::getGyroRateDivisor(void) {
  _setBank(2);

  Adafruit_BusIO_Register gyro_rate_divisor = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_GYRO_SMPLRT_DIV, 1);

  uint8_t divisor_val = gyro_rate_divisor.read();

  _setBank(0);
  return divisor_val;
}

/**************************************************************************/
/*!

    @brief Sets the gyro's data rate divisor.
    @param  new_gyro_divisor
            The gyro's data rate divisor (`uint8_t`).
*/
void Adafruit_ICM20X::setGyroRateDivisor(uint8_t new_gyro_divisor) {
  _setBank(2);

  Adafruit_BusIO_Register gyro_rate_divisor = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_GYRO_SMPLRT_DIV, 1);

  gyro_rate_divisor.write(new_gyro_divisor);
  _setBank(0);
}
/**
 * @brief Sets the polarity of the int1 pin
 *
 * @param active_low Set to true to make INT1 active low, false to make it
 * active high
 */
void Adafruit_ICM20X::setInt1ActiveLow(bool active_low) {

  _setBank(0);

  Adafruit_BusIO_Register int_pin_cfg = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_REG_INT_PIN_CFG);

  Adafruit_BusIO_RegisterBits int1_polarity =
      Adafruit_BusIO_RegisterBits(&int_pin_cfg, 1, 7);

  Adafruit_BusIO_RegisterBits int1_open_drain =
      Adafruit_BusIO_RegisterBits(&int_pin_cfg, 1, 6);

  int1_open_drain.write(true);
  int1_polarity.write(active_low);
}
/**
 * @brief Sets the polarity of the INT2 pin
 *
 * @param active_low Set to true to make INT1 active low, false to make it
 * active high
 */
void Adafruit_ICM20X::setInt2ActiveLow(bool active_low) {

  _setBank(0);

  Adafruit_BusIO_Register int_enable_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_REG_INT_ENABLE_1);

  Adafruit_BusIO_RegisterBits int2_polarity =
      Adafruit_BusIO_RegisterBits(&int_enable_1, 1, 7);

  Adafruit_BusIO_RegisterBits int2_open_drain =
      Adafruit_BusIO_RegisterBits(&int_enable_1, 1, 6);

  int2_open_drain.write(true);
  int2_polarity.write(active_low);
}

/**
 * @brief Sets the bypass status of the I2C master bus support.
 *
 * @param bypass_i2c Set to true to bypass the internal I2C master circuitry,
 * connecting the external I2C bus to the main I2C bus. Set to false to
 * re-connect
 */
void Adafruit_ICM20X::setI2CBypass(bool bypass_i2c) {
  _setBank(0);

  Adafruit_BusIO_Register int_enable_1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_REG_INT_PIN_CFG);

  Adafruit_BusIO_RegisterBits i2c_bypass_enable =
      Adafruit_BusIO_RegisterBits(&int_enable_1, 1, 1);

  i2c_bypass_enable.write(bypass_i2c);
}

/**
 * @brief Enable or disable the I2C mastercontroller
 *
 * @param enable_i2c_master true: enable false: disable
 *
 * @return true: success false: error
 */
bool Adafruit_ICM20X::enableI2CMaster(bool enable_i2c_master) {
  _setBank(0);
  Adafruit_BusIO_Register user_ctrl_reg = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_USER_CTRL);

  Adafruit_BusIO_RegisterBits i2c_master_enable_bit =
      Adafruit_BusIO_RegisterBits(&user_ctrl_reg, 1, 5);
  return i2c_master_enable_bit.write(enable_i2c_master);
}

// TODO: add params
/**
 * @brief Set the I2C clock rate for the auxillary I2C bus to 345.60kHz and
 * disable repeated start
 *
 * @return true: success false: failure
 */
bool Adafruit_ICM20X::_configureI2CMaster(void) {

  uint8_t buffer[2];
  _setBank(3);
  buffer[0] = ICM20X_I2C_MST_CTRL;
  // no repeated start, i2c master clock = 345.60kHz
  buffer[1] = 0x17;
  if (!i2c_dev->write(buffer, 2)) {
    return false;
  }
}
/**
 * @brief Read a single byte to a given register address for an I2C slave device
 * on the auxiliary I2C bus
 *
 * @param slv_addr the 7-bit I2C address of the slave device
 * @param reg_addr the register address to read from
 * @return the requested register value
 */
uint8_t Adafruit_ICM20X::readExternalRegister(uint8_t slv_addr,
                                              uint8_t reg_addr) {

  slv_addr |= 0x80; // set high bit for read

  uint8_t buffer[2];
  uint8_t tries = 0;
  _setBank(3);

  // set the I2C address on the external bus
  buffer[0] = ICM20X_I2C_SLV4_ADDR;
  buffer[1] = slv_addr;
  if (!i2c_dev->write(buffer, 2)) {
    return false;
  }
  // specify which register we're reading
  buffer[0] = ICM20X_I2C_SLV4_REG;
  buffer[1] = reg_addr;
  if (!i2c_dev->write(buffer, 2)) {
    return false;
  }

  // enable the read!
  buffer[0] = ICM20X_I2C_SLV4_CTRL;
  buffer[1] = 0x80;
  if (!i2c_dev->write(buffer, 2)) {
    return false;
  }

  _setBank(0);
  uint8_t addrbuffer[2] = {(uint8_t)ICM20X_I2C_MST_STATUS, (uint8_t)0};
  buffer[0] = 0;
  buffer[1] = 0;
  // wait until the operation is finished
  while (buffer[0] != 0x40) { // this bit is slave4 finished!
    i2c_dev->write_then_read(addrbuffer, 1, buffer, 1);
    tries++;
    if (tries >= NUM_FINISHED_CHECKS) {
      resetI2CMaster();
      return false;
    }
  }
  // read the reading
  _setBank(3);
  addrbuffer[0] = (uint8_t)ICM20X_I2C_SLV4_DI;
  buffer[0] = 0;
  buffer[1] = 0;
  i2c_dev->write_then_read(addrbuffer, 1, buffer, 1);
  return buffer[0];
}

/**
 * @brief Write a single byte to a given register address for an I2C slave
 * device on the auxiliary I2C bus
 *
 * @param slv_addr the 7-bit I2C address of the slave device
 * @param reg_addr the register address to write to
 * @param value the value to write
 * @return true
 * @return false
 */
bool Adafruit_ICM20X::writeExternalRegister(uint8_t slv_addr, uint8_t reg_addr,
                                            uint8_t value) {
  uint8_t buffer[2];

  // Adafruit_BusIO_Register slv4_addr = Adafruit_BusIO_Register(
  //   i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_I2C_SLV4_ADDR);

  // Adafruit_BusIO_Register slv4_addr = Adafruit_BusIO_Register(
  //   i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_I2C_SLV4_REG);

  // Adafruit_BusIO_Register slv4_addr = Adafruit_BusIO_Register(
  //   i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_I2C_SLV4_DO);

  // Adafruit_BusIO_Register slv4_addr = Adafruit_BusIO_Register(
  //   i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_I2C_SLV4_CTRL);

  _setBank(3);
  uint8_t tries = 0;
  // set the I2C address on the external bus
  buffer[0] = ICM20X_I2C_SLV4_ADDR;
  buffer[1] = slv_addr;
  if (!i2c_dev->write(buffer, 2)) {
    Serial.println("bad addr set");
    return false;
  }
  // specify which register we're writing to
  buffer[0] = ICM20X_I2C_SLV4_REG;
  buffer[1] = reg_addr;
  if (!i2c_dev->write(buffer, 2)) {
    Serial.println("bad reg set");
    return false;
  }
  // set the data to write
  buffer[0] = ICM20X_I2C_SLV4_DO;
  buffer[1] = value;
  if (!i2c_dev->write(buffer, 2)) {
    Serial.println("bad DO set");
    return false;
  }
  // start writing
  buffer[0] = ICM20X_I2C_SLV4_CTRL;
  buffer[1] = 0x80;
  if (!i2c_dev->write(buffer, 2)) {
    Serial.println("bad CTRL set");
    return false;
  }

  _setBank(0);
  uint8_t addrbuffer[2] = {(uint8_t)ICM20X_I2C_MST_STATUS, (uint8_t)0};
  buffer[0] = 0;
  buffer[1] = 0;
  // wait until the operation is finished
  while (buffer[0] != 0x40) {
    i2c_dev->write_then_read(addrbuffer, 1, buffer, 1);
    tries++;
    if (tries >= NUM_FINISHED_CHECKS) {
      Serial.println("failed to get i2c master finished signal");
      return false;
    }
  }
  return true;
}
/**
 * @brief Reset the I2C master
 *
 */
void Adafruit_ICM20X::resetI2CMaster(void) {

  _setBank(0);
  Adafruit_BusIO_Register user_ctrl = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, ICM20X_USER_CTRL);

  Adafruit_BusIO_RegisterBits i2c_master_reset_bit =
      Adafruit_BusIO_RegisterBits(&user_ctrl, 1, 1);

  i2c_master_reset_bit.write(true);
  while (i2c_master_reset_bit.read()) {
    delay(10);
  }
  delay(100);
}

/**************************************************************************/
/*!
    @brief  Gets the sensor_t data for the ICM20X's accelerometer
*/
/**************************************************************************/
void Adafruit_ICM20X_Accelerometer::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "ICM20X_A", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_ACCELEROMETER;
  sensor->min_delay = 0;
  sensor->min_value = -294.1995F; /*  -30g = 294.1995 m/s^2  */
  sensor->max_value = 294.1995F;  /* 30g = 294.1995 m/s^2  */
  sensor->resolution =
      0.122; /* 8192LSB/1000 mG -> 8.192 LSB/ mG => 0.122 mG/LSB at +-4g */
}

/**************************************************************************/
/*!
    @brief  Gets the accelerometer as a standard sensor event
    @param  event Sensor event object that will be populated
    @returns True
*/
/**************************************************************************/
bool Adafruit_ICM20X_Accelerometer::getEvent(sensors_event_t *event) {
  _theICM20X->_read();
  _theICM20X->fillAccelEvent(event, millis());

  return true;
}

/**************************************************************************/
/*!
    @brief  Gets the sensor_t data for the ICM20X's gyroscope sensor
*/
/**************************************************************************/
void Adafruit_ICM20X_Gyro::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "ICM20X_G", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_GYROSCOPE;
  sensor->min_delay = 0;
  sensor->min_value = -69.81; /* -4000 dps -> rad/s (radians per second) */
  sensor->max_value = +69.81;
  sensor->resolution = 2.665e-7; /* 65.5 LSB/DPS */
}

/**************************************************************************/
/*!
    @brief  Gets the gyroscope as a standard sensor event
    @param  event Sensor event object that will be populated
    @returns True
*/
/**************************************************************************/
bool Adafruit_ICM20X_Gyro::getEvent(sensors_event_t *event) {
  _theICM20X->_read();
  _theICM20X->fillGyroEvent(event, millis());

  return true;
}

/**************************************************************************/
/*!
    @brief  Gets the sensor_t data for the ICM20X's magnetometer sensor
*/
/**************************************************************************/
void Adafruit_ICM20X_Magnetometer::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "ICM20X_M", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_MAGNETIC_FIELD;
  sensor->min_delay = 0;
  sensor->min_value = -4900;
  sensor->max_value = 4900;
  sensor->resolution = 0.6667;
}

/**************************************************************************/
/*!
    @brief  Gets the magnetometer as a standard sensor event
    @param  event Sensor event object that will be populated
    @returns True
*/
/**************************************************************************/
bool Adafruit_ICM20X_Magnetometer::getEvent(sensors_event_t *event) {
  _theICM20X->_read();
  _theICM20X->fillMagEvent(event, millis());

  return true;
}

/**************************************************************************/
/*!
    @brief  Gets the sensor_t data for the ICM20X's tenperature
*/
/**************************************************************************/
void Adafruit_ICM20X_Temp::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "ICM20X_T", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  sensor->min_delay = 0;
  sensor->min_value = -40;
  sensor->max_value = 85;
  sensor->resolution = 0.0029952; /* 333.87 LSB/C => 1/333.87 C/LSB */
}

/**************************************************************************/
/*!
    @brief  Gets the temperature as a standard sensor event
    @param  event Sensor event object that will be populated
    @returns True
*/
/**************************************************************************/
bool Adafruit_ICM20X_Temp::getEvent(sensors_event_t *event) {
  _theICM20X->_read();
  _theICM20X->fillTempEvent(event, millis());

  return true;
}
