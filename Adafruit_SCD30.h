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
  // "requires_busio": "y",
  //   "requires_sensor": "y",
#ifndef _ADAFRUIT_SCD30_H
#define _ADAFRUIT_SCD30_H

#include "Arduino.h"
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>#define SCD30_I2CADDR_DEFAULT 0x60 ///< SCD30 default i2c address
#define SCD30_CHIP_ID 0x60 ///< SCD30 default device id from WHOAMI

#define SCD30_WHOAMI 0xD100 ///< Chip ID register

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

/** Adafruit Unified Sensor interface for the pressure sensor component of SCD30
 */
class Adafruit_SCD30_Pressure : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the pressure sensor
      @param parent A pointer to the SCD30 class */
  Adafruit_SCD30_Pressure(Adafruit_SCD30 *parent) { _theSCD30 = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0xC02+1;
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

  bool begin_SPI(uint8_t cs_pin, SPIClass *theSPI = &SPI,
                 int32_t sensor_id = 0);
  bool begin_SPI(int8_t cs_pin, int8_t sck_pin, int8_t miso_pin,
                 int8_t mosi_pin, int32_t sensor_id = 0);
  
  void reset(void);
  void interruptsActiveLow(bool active_low);

  
  scd30_rate_t getDataRate(void);

  void setDataRate(scd30_rate_t data_rate);
  bool getEvent(sensors_event_t *pressure, sensors_event_t *temp);

  Adafruit_Sensor *getTemperatureSensor(void);
  Adafruit_Sensor *getPressureSensor(void);

protected:
  void _read(void);
  virtual bool _init(int32_t sensor_id);

  float unscaled_temp,   ///< Last reading's temperature (C) before scaling
      unscaled_pressure; ///< Last reading's pressure (hPa) before scaling

  uint16_t _sensorid_pressure, ///< ID number for pressure
      _sensorid_temp;          ///< ID number for temperature

  Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface
  Adafruit_SPIDevice *spi_dev = NULL; ///< Pointer to SPI bus interface

  Adafruit_SCD30_Temp *temp_sensor = NULL; ///< Temp sensor data object
  Adafruit_SCD30_Pressure *pressure_sensor =
      NULL; ///< Pressure sensor data object

private:
  friend class Adafruit_SCD30_Temp;     ///< Gives access to private members to
                                        ///< Temp data object
  friend class Adafruit_SCD30_Pressure; ///< Gives access to private
                                        ///< members to Pressure data
                                        ///< object

  void fillPressureEvent(sensors_event_t *pressure, uint32_t timestamp);
  void fillTempEvent(sensors_event_t *temp, uint32_t timestamp);
};


#endif
