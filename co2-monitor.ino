#include "battery.h"
#include "co2.h"

battery::Task *battery_task;
co2::Task *co2_task;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  esp_sleep_enable_timer_wakeup(10e6);

  battery_task = new battery::FakeTask();
  co2_task = new co2::FakeTask();
}

void loop() {
  // put your main code here, to run repeatedly:
  co2::Result co2_result = co2_task->run();
  String msg = "co2: ";
  msg += co2_result.co2;
  Serial.println(msg);

  battery::Result battery_result = battery_task->run();
  msg = "battery: ";
  msg += battery_result.voltage;
  msg += " ";
  msg += battery_result.percent;
  Serial.println(msg);

  Serial.println("going to sleep...");
  esp_deep_sleep_start();
  Serial.println("woke up...");
}
