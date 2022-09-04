#include <Adafruit_NeoPixel.h>
#include "battery.h"
#include "co2.h"

battery::Task *battery_task;
co2::Task *co2_task;
bool i2c_power_polarity;

Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

void wait_for_tasks() {
    Serial.println("going to sleep while tasks are running...");
    delay(1000);
}

void wait_for_next_cycle() {
    Serial.println("going to sleep till next cycle...");
    esp_sleep_enable_timer_wakeup(10 * 1e6);
    esp_deep_sleep_start();
}

void enable_i2c_power() {
  pinMode(PIN_I2C_POWER, INPUT);
  delay(1);
  i2c_power_polarity = digitalRead(PIN_I2C_POWER);
  pinMode(PIN_I2C_POWER, OUTPUT);
  digitalWrite(PIN_I2C_POWER, !i2c_power_polarity);
}

void disable_i2c_power() {
  digitalWrite(PIN_I2C_POWER, i2c_power_polarity);
  pinMode(PIN_I2C_POWER, INPUT);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  enable_i2c_power();
  Wire.begin();

  battery_task = new battery::LC709203FTask();
  co2_task = new co2::SCD4xTask();

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(10);
  pixels.setPixelColor(0, pixels.Color(255, 255, 0));
  pixels.show();
}

void loop() {
  co2::Result co2_result = co2_task->run();
  String msg = "co2: " + String(co2_result.co2);
  Serial.println(msg);

  battery::Result battery_result = battery_task->run();
  msg = "battery: " + String(battery_result.voltage) + " " + String(battery_result.percent);
  Serial.println(msg);

  int red = 0;
  int green = 0;
  int blue = 0;

  if (battery_result.state == battery::DONE) {
    blue = 255;
  }

  if (co2_result.state == co2::DONE) {
    green = 255;
  }

  if (red || blue || green) {
      pixels.setPixelColor(0, pixels.Color(red, green, blue));
    pixels.show();
  }

  if (co2_result.state != co2::DONE || battery_result.state != battery::DONE) {
    wait_for_tasks();

  } else {
    Serial.println("i2c tasks done - disabling i2c power...");
    delay(2000); //TODO update display
    disable_i2c_power();
    wait_for_next_cycle();
  }
}
