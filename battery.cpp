#include "battery.h"

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

}
