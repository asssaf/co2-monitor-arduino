#include "battery.h"
#include "Adafruit_LC709203F.h"

namespace battery {

  Result FakeTask::run() {
    switch (this->result.state) {
    case INIT:
      this->result.state = MEASURING;
      break;

    case MEASURING:
      this->result.state = DONE;
      this->result.voltage = 5;
      this->result.percent = 0.85;
      break;

    case DONE:
      break;
    }

    return result;
  }


  Result LC709203FTask::run() {
    switch (this->result.state) {
    case INIT:
      if (!lc.begin()) {
        Serial.println("failed to initialize battery monitor");
        this->result.state = DONE;
        break;
      }
      lc.setPackSize(LC709203F_APA_500MAH);

      this->result.state = MEASURING;
      break;

    case MEASURING:
      this->result.state = DONE;
      this->result.voltage = lc.cellVoltage();
      this->result.percent = lc.cellPercent();
      break;

    case DONE:
      break;
    }

    return result;
  }

}
