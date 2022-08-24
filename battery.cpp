#include "battery.h"

namespace battery {

  Result FakeTask::run() {
    return Result {
      voltage: 3.7,
      percent: 0.85
    };
  }

}
