#include <Adafruit_NeoPixel.h>
#include <Adafruit_ThinkInk.h>
#include <soc/usb_serial_jtag_struct.h>

#include "battery.h"
#include "co2.h"

#define EPD_DC      10 // can be any pin, but required!
#define EPD_CS      9  // can be any pin, but required!
#define EPD_BUSY    18 // can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS     -1  // can set to -1 to not use a pin (uses a lot of RAM!)
#define EPD_RESET   -1 // can set to -1 and share with chip Reset (can't deep sleep)

#define CALIBRATION_PIN 11 // 13 is connected to the builtin led so use 11

#define DISPLAY_ENABLED 1

struct DisplayData {
  using BatteryResult = battery::Result;
  using CO2Result = co2::Result;
  BatteryResult battery;
  CO2Result co2;
};

battery::Task *battery_task;
co2::Task *co2_task;
bool i2c_power_polarity;
unsigned long start_millis = 0;
unsigned long sleep_millis = 0;
bool calibration_mode = false;

Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
ThinkInk_290_Grayscale4_T5 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

bool is_connected() {
  return USB_SERIAL_JTAG.fram_num.sof_frame_index > 0;
}

void wait_for_tasks() {
    Serial.println("going to sleep while tasks are running...");
    unsigned long sleep_micros = 250 * 1e3;
    esp_sleep_enable_timer_wakeup(sleep_micros);
    esp_light_sleep_start();
    sleep_millis += sleep_micros/1000;
}

void wait_for_next_cycle() {
    unsigned long total_millis = millis()-start_millis;
    unsigned long active_millis = total_millis - sleep_millis;
    Serial.println("total_ms=" + String(total_millis) + ", active_ms=" + String(active_millis) + ", sleep_ms=" + String(sleep_millis));
    Serial.println("going to sleep till next cycle...");
    if (is_connected()) {
      if (DISPLAY_ENABLED) {
        esp_sleep_enable_timer_wakeup(180 * 1e6);
      } else {
        esp_sleep_enable_timer_wakeup(10 * 1e6);
      }
      Serial.println("pretending to deep sleep");
      Serial.flush();
      esp_light_sleep_start();
      ESP.restart();

    } else {
      esp_sleep_enable_timer_wakeup(300 * 1e6);
      esp_deep_sleep_start();
    }
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

void display_anchored_text(String str, int x, int y, float ax, float ay) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(-ax*w + x, -ay*h + y);
  display.print(str);
}

void update_display(DisplayData data) {
  display.begin(THINKINK_MONO);

  int16_t width = display.width();
  int16_t height = display.height();

  display.clearBuffer();

  display.setTextColor(EPD_BLACK);
  display.setTextSize(3);

  String co2 = String(data.co2.co2);
  if (calibration_mode) {
    co2 += " c: " + String(data.co2.correction);
  }
  display_anchored_text(co2, width/2, height/2, 0.5, 0.5);

  display.setTextSize(2);
  String battery_str = String(data.battery.voltage, 2) + "V " + String(data.battery.percent, 1) + "%";
  display_anchored_text(battery_str, 10, height-10, 0.0, 1.0);

  String temperature_str = String(data.co2.temperature, 1) + "C";
  display_anchored_text(temperature_str, 10, 10, 0.0, 0.0);

  String humidity_str = String(data.co2.humidity, 1) + "%";
  display_anchored_text(humidity_str, width-10, 10, 1.0, 0.0);

  display.display(true /* power down after */);
}

void setup() {
  start_millis = millis();

  // put your setup code here, to run once:
  Serial.begin(115200);

  enable_i2c_power();
  Wire.begin();

  // check if calibration button is pressed
  pinMode(CALIBRATION_PIN, INPUT_PULLUP);
  calibration_mode = (digitalRead(CALIBRATION_PIN) == LOW);
  Serial.println("calibration mode: " + String(calibration_mode));

  battery_task = new battery::LC709203FTask();
  co2_task = new co2::SCD4xTask(calibration_mode);

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
  } else if (calibration_mode) {
    red = 255;
    green = 0;
    blue = 255;
  }

  if (red || blue || green) {
    pixels.setPixelColor(0, pixels.Color(red, green, blue));
    pixels.show();
  }

  if (co2_result.state != co2::DONE || battery_result.state != battery::DONE) {
    wait_for_tasks();

  } else {
    Serial.println("i2c tasks done - disabling i2c power...");
    disable_i2c_power();

    pixels.setPixelColor(0, pixels.Color(255, 255, 0));
    pixels.show();

    DisplayData display_data = DisplayData{
      battery: battery_result,
      co2: co2_result,
    };

    if (DISPLAY_ENABLED) {
      update_display(display_data);
    }

    wait_for_next_cycle();
  }
}
