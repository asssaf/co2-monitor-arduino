#include "battery.h"

namespace battery {

  Result FakeMonitor::fetch() {
    return Result {
      voltage: 3.7,
      percent: 0.85
    };
  }

}
