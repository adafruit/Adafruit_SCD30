


// Basic demo for readings from Adafruit SCD30
#include <Wire.h>
#include <Adafruit_SCD30.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>
// For SPI mode, we need a CS pin
#define SCD30_CS 10
// For software-SPI mode we need SCK/MOSI/MISO pins
#define SCD30_SCK 13
#define SCD30_MISO 12
#define SCD30_MOSI 11

Adafruit_SCD30  scd30;

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
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


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(500); // Pause for half second

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(0);

}
void loop() {

  if(scd30.dataReady()){
    Serial.println("Data available!");
    sensors_event_t temp;
    sensors_event_t humidity;

    display.clearDisplay();
    display.setCursor(0,0);

    scd30.getEvent(&humidity, &temp);// get humidity
    Serial.print("Temperature: ");Serial.print(temp.temperature);Serial.println(" degrees C");
    Serial.print("Relative Humidity: ");Serial.print(humidity.relative_humidity);Serial.println(" %");
    Serial.print("eCO2: ");Serial.print(scd30.eCO2, 3);Serial.println(" ppm");
    Serial.println("");

    display.print("Temp: ");display.print(scd30.temperature);display.println(" C");
    display.print("Rel Hum: ");display.print(scd30.relative_humidity);display.println(" %rH");
    display.print("eCO2: "); display.print(scd30.eCO2, 3); display.println(" ppm");
    display.display();
  }

  delay(100);

}