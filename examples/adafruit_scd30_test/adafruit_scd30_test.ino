// Basic demo for readings from Adafruit SCD30
#include <Wire.h>
#include <Adafruit_SCD30.h>
#include <Adafruit_Sensor.h>

// For SPI mode, we need a CS pin
#define SCD30_CS 10
// For software-SPI mode we need SCK/MOSI/MISO pins
#define SCD30_SCK 13
#define SCD30_MISO 12
#define SCD30_MOSI 11

Adafruit_SCD30  scd30; // TODO FIX NAME
void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit SCD30 test!");

  // Try to initialize!
  if (!scd30.begin_I2C()) {
  //if (!scd30.begin_SPI(SCD30_CS)) {
  //if (!scd30.begin_SPI(SCD30_CS, SCD30_SCK, SCD30_MISO, SCD30_MOSI)) {
    Serial.println("Failed to find SCD30 chip");
    while (1) { delay(10); }
  }
  Serial.println("SCD30 Found!");

// //  scd30.setDataRate(SCD30_RATE_12_5_HZ);
//   Serial.print("Data rate set to: ");
//   switch (scd30.getDataRate()) {
//     case SCD30_RATE_ONE_SHOT: Serial.println("One Shot"); break;
//     case SCD30_RATE_1_HZ: Serial.println("1 Hz"); break;
//     case SCD30_RATE_7_HZ: Serial.println("7 Hz"); break;

//   }
}
void loop() {

  sensors_event_t temp;
  sensors_event_t humidity;
  scd30.getEvent(&humidity, &temp);// get humidity
  Serial.print("Temperature: ");Serial.print(temp.temperature);Serial.println(" degrees C");
  Serial.print("Relative Humidity: ");Serial.print(humidity.relative_humidity);Serial.println(" %");
  Serial.println("");
  delay(100);

}
