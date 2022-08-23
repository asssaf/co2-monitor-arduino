#include "co2.h"

namespace co2 {

  Result FakeMonitor::fetch() {
    return Result { 5.0 };
  }

}
