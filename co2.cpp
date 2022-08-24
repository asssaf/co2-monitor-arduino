#include "co2.h"

namespace co2 {

  Result FakeTask::run() {
    switch (this->state) {
    case INIT:
      this->state = MEASURING;
      return Result {
        state: this->state,
      };
    case MEASURING:
      this->state = DONE;
      return Result {
        state: this->state,
        co2: 5.0
      };

    case DONE:
      return Result{
        state: this->state,
      };
    }
  }

}
