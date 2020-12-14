/*!
 *  @file Adafruit_SCD30.h
 *
 * 	I2C Driver for the Adafruit SCD30 eCO2, Temperature, and Humidity sensor
 * 	This is a library is written to work with the Adafruit SCD30 breakout:
 * 	https://www.adafruit.com/products/48xx
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *
 *	 license (see license.txt)
 */

#ifndef _ADAFRUIT_SCD30_H
#define _ADAFRUIT_SCD30_H

#include "Arduino.h"
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#define SCD30_I2CADDR_DEFAULT 0x61 ///< SCD30 default i2c address
#define SCD30_CHIP_ID 0x60         ///< SCD30 default device id from WHOAMI

#define SCD30_WHOAMI 0xD100 ///< Chip ID register

#define SCD30_CMD_READ_MEASUREMENT 0x0300 ///< Main data register

// ************ NOTE! The addresses below have their MSB/LSB swapped to fit with
// Busio
#define SCD30_CMD_CONTINUOUS_MEASUREMENT                                       \
  0x1000 ///< Command to start continuous measurement
#define SCD30_CMD_SET_MEASUREMENT_INTERVAL                                     \
  0x0046                                ///< Command to set measurement intercal
#define SCD30_CMD_GET_DATA_READY 0x0202 ///< Data ready reg
#define SCD30_CMD_AUTOMATIC_SELF_CALIBRATION                                   \
  0x0653 ///< enables/disables auto calibration
#define SCD30_CMD_SET_FORCED_RECALIBRATION_REF                                 \
  0x0452 ///< Forces calibration with given value
#define SCD30_CMD_SET_TEMPERATURE_OFFSET 0x0354 ///< Specifies the temp offset
#define SCD30_CMD_SET_ALTITUDE_COMPENSATION                                    \
  0x0251                               ///< Specifies altitude offset
#define SCD30_CMD_SOFT_RESET 0x04D3    ///< Soft reset!
#define SCD30_CMD_READ_REVISION 0x00D1 ///< Firmware revision number
///////////////////////////////////////////////////////////////
/**
 * @brief
 *
 * Allowed values for `setDataRate`.
 */
typedef enum {
  SCD30_RATE_ONE_SHOT,
  SCD30_RATE_1_HZ,
  SCD30_RATE_7_HZ,
  SCD30_RATE_12_5_HZ,
  SCD30_RATE_25_HZ,
} scd30_rate_t;

class Adafruit_SCD30;

/** Adafruit Unified Sensor interface for temperature component of SCD30 */
class Adafruit_SCD30_Temp : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the temp sensor
      @param parent A pointer to the SCD30 class */
  Adafruit_SCD30_Temp(Adafruit_SCD30 *parent) { _theSCD30 = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0xC02;
  Adafruit_SCD30 *_theSCD30 = NULL;
};

/** Adafruit Unified Sensor interface for the humidity sensor component of SCD30
 */
class Adafruit_SCD30_Humidity : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the humidity sensor
      @param parent A pointer to the SCD30 class */
  Adafruit_SCD30_Humidity(Adafruit_SCD30 *parent) { _theSCD30 = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0xC02 + 1;
  Adafruit_SCD30 *_theSCD30 = NULL;
};

/*!
 *    @brief  Class that stores state and functions for interacting with
 *            the SCD30 eCO2, Temperature, and Humidity sensor
 */
class Adafruit_SCD30 {
public:
  Adafruit_SCD30();
  ~Adafruit_SCD30();

  bool begin_I2C(uint8_t i2c_addr = SCD30_I2CADDR_DEFAULT,
                 TwoWire *wire = &Wire, int32_t sensor_id = 0);

  void reset(void);
  bool dataReady(void);
  bool getEvent(sensors_event_t *humidity, sensors_event_t *temp);

  Adafruit_Sensor *getTemperatureSensor(void);
  Adafruit_Sensor *getHumiditySensor(void);
  float eCO2; ///< The most recent eCO2 reading

protected:
  bool _read(void);
  virtual bool _init(int32_t sensor_id);

  float unscaled_temp,   ///< Last reading's temperature (C) before scaling
      unscaled_humidity; ///< Last reading's humidity (%rH) before scaling

  uint16_t _sensorid_humidity, ///< ID number for humidity
      _sensorid_temp;          ///< ID number for temperature

  Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface

  Adafruit_SCD30_Temp *temp_sensor = NULL; ///< Temp sensor data object
  Adafruit_SCD30_Humidity *humidity_sensor =
      NULL; ///< Humidity sensor data object

private:
  friend class Adafruit_SCD30_Temp;     ///< Gives access to private members to
                                        ///< Temp data object
  friend class Adafruit_SCD30_Humidity; ///< Gives access to private
                                        ///< members to Humidity data
                                        ///< object

  void fillHumidityEvent(sensors_event_t *humidity, uint32_t timestamp);
  void fillTempEvent(sensors_event_t *temp, uint32_t timestamp);
  bool sendCommand(uint16_t command, uint16_t argument);
  bool sendCommand(uint16_t command);
  uint8_t computeCRC8(uint8_t data[], uint8_t len);
  uint16_t readRegister(uint16_t reg_address);
  float temperature, relative_humidity;
  // static uint8_t crc8(const uint8_t *data, int len);
};

static uint8_t crc8(const uint8_t *data, int len);

#endif
