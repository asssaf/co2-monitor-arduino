#include "co2.h"

co2::Monitor *co2_monitor;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  co2_monitor = new co2::FakeMonitor();
}

void loop() {
  // put your main code here, to run repeatedly:
  co2::Result result = co2_monitor->fetch();
  String msg = "result: ";
  msg += result.co2;
  Serial.println(msg);
}
