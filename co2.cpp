#include "co2.h"

namespace co2 {

  Result FakeTask::run() {
    switch (this->result.state) {
    case INIT:
      this->result.state = MEASURING;
      break;

    case MEASURING:
      this->result.state = DONE;
      this->result.co2 = 5;
      break;

    case DONE:
      break;
    }

    return result;
  }

}
