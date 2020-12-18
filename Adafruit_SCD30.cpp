/*!
 *  @file Adafruit_SCD30.cpp
 *
 *  @mainpage Adafruit SCD30 eCO2, Temperature, and Humidity sensor
 *
 *  @section intro_sec Introduction
 *
 * 	I2C Driver for the Library for the SCD30 eCO2, Temperature, and Humidity
 * sensor
 *
 * 	This is a library for the Adafruit SCD30 breakout:
 * 	https://www.adafruit.com/product/48xx
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

#include "Adafruit_SCD30.h"

/**
 * @brief Construct a new Adafruit_SCD30::Adafruit_SCD30 object
 *
 */
Adafruit_SCD30::Adafruit_SCD30(void) {}

/**
 * @brief Destroy the Adafruit_SCD30::Adafruit_SCD30 object
 *
 */
Adafruit_SCD30::~Adafruit_SCD30(void) {
  if (temp_sensor)
    delete temp_sensor;
  if (humidity_sensor)
    delete humidity_sensor;
}

/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  i2c_address
 *            The I2C address to be used.
 *    @param  wire
 *            The Wire object to be used for I2C connections.
 *    @param  sensor_id
 *            The unique ID to differentiate the sensors from others
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_SCD30::begin_I2C(uint8_t i2c_address, TwoWire *wire,
                               int32_t sensor_id) {
  if (i2c_dev) {
    delete i2c_dev; // remove old interface
  }

  i2c_dev = new Adafruit_I2CDevice(i2c_address, wire);

  if (!i2c_dev->begin()) {
    return false;
  }

  return _init(sensor_id);
}
// bool Adafruit_SCD30::begin_UART(void){}

/*!  @brief Initializer for post i2c/spi init
 *   @param sensor_id Optional unique ID for the sensor set
 *   @returns True if chip identified and initialized
 */
bool Adafruit_SCD30::_init(int32_t sensor_id) {

  // uint16_t chip_id = sendCommand(SCD30_CMD_)
  _sensorid_humidity = sensor_id;
  _sensorid_temp = sensor_id + 1;

  // reset();
  // do any software reset or other initial setup

  // // Set to highest rate
  // setDataRate(SCD30_RATE_25_HZ);

  sendCommand(SCD30_CMD_CONTINUOUS_MEASUREMENT, 0);
  sendCommand(SCD30_CMD_SET_MEASUREMENT_INTERVAL, 2);
  sendCommand(SCD30_CMD_AUTOMATIC_SELF_CALIBRATION, 1);
  // humidity_sensor = new Adafruit_SCD30_Humidity(this);
  // temp_sensor = new Adafruit_SCD30_Temp(this);
  return true;
}

/**
 * @brief Performs a software reset initializing registers to their power on
 * state
 *
 */
void Adafruit_SCD30::reset(void) {
  sendCommand(SCD30_CMD_SOFT_RESET);
  delay(30);
}

bool Adafruit_SCD30::sendCommand(uint16_t command) {
  Adafruit_BusIO_Register _command =
      Adafruit_BusIO_Register(i2c_dev, command, 2, MSBFIRST, 2);
  uint8_t buffer[3];

  return _command.write(
      buffer,
      0); // may not work? should send command/reg, then buffer as one txn
}
bool Adafruit_SCD30::sendCommand(uint16_t command, uint16_t argument) {
  Adafruit_BusIO_Register _command =
      Adafruit_BusIO_Register(i2c_dev, command, 2, MSBFIRST, 2);
  uint8_t buffer[3];
  buffer[0] = argument >> 8;
  buffer[1] = argument & 0xFF;
  buffer[2] = crc8(buffer, 2);
  _command.write(
      buffer,
      3); // may not work? should send command/reg, then buffer as one txn
}
uint16_t Adafruit_SCD30::readRegister(uint16_t reg_address) {
  uint8_t buffer[2];
  buffer[0] = (reg_address >> 8) & 0xFF;
  buffer[1] = reg_address & 0xFF;
  // the SCD30 really wants a stop before the read!
  i2c_dev->write_then_read(buffer, 2, buffer, 2, true);
  return (uint16_t)(buffer[0] << 8 | (buffer[1] & 0xFF));
}

/**
 * @brief Ask the sensor if new data is ready to read
 *
 * @return true: data is available
 * @return false no new data available
 */
bool Adafruit_SCD30::dataReady(void) {
  return (readRegister(SCD30_CMD_GET_DATA_READY) == 1);
}

/**
 * @brief Set the amount of time between measurements
 *
 * @param interval The time between measurements in seconds
 * @return true: success false: failure
 */
bool Adafruit_SCD30::setMeasurementInterval(uint16_t interval) {
  if ((interval < 2) || (interval > 1800)) {
    return false;
  }
  return sendCommand(SCD30_CMD_SET_MEASUREMENT_INTERVAL, interval);
}

/**
 * @brief Read the current amount of time between measurements
 *
 * @return uint16_t The current measurement interval in seconds.
 */
uint16_t Adafruit_SCD30::getMeasurementInterval(void) {}
/**
 * @brief  Updates the measurement data for all sensors simultaneously
 *
 * @return true: success false: failure
 */
bool Adafruit_SCD30::read(void) {

  uint8_t buffer[18];
  uint8_t crc = 0;

  buffer[0] = (SCD30_CMD_READ_MEASUREMENT >> 8) & 0xFF;
  buffer[1] = SCD30_CMD_READ_MEASUREMENT & 0xFF;
  // This should put a stop between write and read which I believe is necessary
  i2c_dev->write_then_read(buffer, 2, buffer, 18, true);

  uint8_t crc_buffer[2];
  // loop through the bytes we read, 3 at a time for i=MSB, i+1=LSB, i+2=CRC
  for (uint8_t i = 0; i < 18; i += 3) {
    if (crc8(buffer + i, 2) != buffer[i + 2]) {
      // we got a bad CRC, fail out
      return false;
    }
  }
  // CRCs are good, unpack floats
  uint32_t co2, temp, hum;

  co2 |= buffer[0];
  co2 <<= 8;
  co2 |= buffer[1];
  co2 <<= 8;
  co2 |= buffer[3];
  co2 <<= 8;
  co2 |= buffer[4];

  temp |= buffer[6];
  temp <<= 8;
  temp |= buffer[7];
  temp <<= 8;
  temp |= buffer[9];
  temp <<= 8;
  temp |= buffer[10];

  hum |= buffer[12];
  hum <<= 8;
  hum |= buffer[13];
  hum <<= 8;
  hum |= buffer[15];
  hum <<= 8;
  hum |= buffer[16];

  memcpy(&eCO2, &co2, sizeof(eCO2));
  memcpy(&temperature, &temp, sizeof(temperature));
  memcpy(&relative_humidity, &hum, sizeof(relative_humidity));

  return true;
}

/*!
    @brief  Gets an Adafruit Unified Sensor object for the presure sensor
   component
    @return Adafruit_Sensor pointer to humidity sensor
 */
Adafruit_Sensor *Adafruit_SCD30::getHumiditySensor(void) {
  return humidity_sensor;
}

/*!
    @brief  Gets an Adafruit Unified Sensor object for the temp sensor component
    @return Adafruit_Sensor pointer to temperature sensor
 */
Adafruit_Sensor *Adafruit_SCD30::getTemperatureSensor(void) {
  return temp_sensor;
}

/**************************************************************************/
/*!
    @brief  Gets the humidity sensor and temperature values as sensor events
    @param  humidity Sensor event object that will be populated with humidity
   data
    @param  temp Sensor event object that will be populated with temp data
    @returns True
*/
/**************************************************************************/
bool Adafruit_SCD30::getEvent(sensors_event_t *humidity,
                              sensors_event_t *temp) {
  uint32_t t = millis();
  read();

  // use helpers to fill in the events
  fillHumidityEvent(humidity, t);
  fillTempEvent(temp, t);
  return true;
}

void Adafruit_SCD30::fillHumidityEvent(sensors_event_t *humidity,
                                       uint32_t timestamp) {
  memset(humidity, 0, sizeof(sensors_event_t));
  humidity->version = sizeof(sensors_event_t);
  humidity->sensor_id = _sensorid_humidity;
  humidity->type = SENSOR_TYPE_RELATIVE_HUMIDITY;
  humidity->timestamp = timestamp;
  humidity->relative_humidity = relative_humidity;
}

void Adafruit_SCD30::fillTempEvent(sensors_event_t *temp, uint32_t timestamp) {
  memset(temp, 0, sizeof(sensors_event_t));
  temp->version = sizeof(sensors_event_t);
  temp->sensor_id = _sensorid_temp;
  temp->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  temp->timestamp = timestamp;
  temp->temperature = temperature;
}

/**************************************************************************/
/*!
    @brief  Gets the sensor_t data for the SCD30's tenperature
*/
/**************************************************************************/
void Adafruit_SCD30_Humidity::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "SCD30_P", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_PRESSURE;
  sensor->min_delay = 0;
  sensor->min_value = 260;
  sensor->max_value = 1260;
  // 4096 LSB = 1 hPa >>  1 LSB = 1/4096 hPa >> 1 LSB =  2.441e-4 hPa
  sensor->resolution = 2.441e-4;
}

/**************************************************************************/
/*!
    @brief  Gets the humidity as a standard sensor event
    @param  event Sensor event object that will be populated
    @returns True
*/
/**************************************************************************/
bool Adafruit_SCD30_Humidity::getEvent(sensors_event_t *event) {
  _theSCD30->read();
  _theSCD30->fillHumidityEvent(event, millis());

  return true;
}

/**************************************************************************/
/*!
    @brief  Gets the sensor_t data for the SCD30's tenperature
*/
/**************************************************************************/
void Adafruit_SCD30_Temp::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "SCD30_T", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  sensor->min_delay = 0;
  sensor->min_value = -30;
  sensor->max_value = 105;
  // 480 LSB = 1°C >> 1 LSB = 1/480°C >> 1 LSB =  0.00208 °C
  sensor->resolution = 0.00208;
}

/**************************************************************************/
/*!
    @brief  Gets the temperature as a standard sensor event
    @param  event Sensor event object that will be populated
    @returns True
*/
/**************************************************************************/
bool Adafruit_SCD30_Temp::getEvent(sensors_event_t *event) {
  _theSCD30->read();
  _theSCD30->fillTempEvent(event, millis());

  return true;
}

/**
 * Performs a CRC8 calculation on the supplied values.
 *
 * @param data  Pointer to the data to use when calculating the CRC8.
 * @param len   The number of bytes in 'data'.
 *
 * @return The computed CRC8 value.
 */
static uint8_t crc8(const uint8_t *data, int len) {
  /*
   *
   * CRC-8 formula from page 14 of SHT spec pdf
   *
   * Test data 0xBE, 0xEF should yield 0x92
   *
   * Initialization data 0xFF
   * Polynomial 0x31 (x8 + x5 +x4 +1)
   * Final XOR 0x00
   */

  const uint8_t POLYNOMIAL(0x31);
  uint8_t crc(0xFF);

  for (int j = len; j; --j) {
    crc ^= *data++;

    for (int i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    }
  }
  return crc;
}
