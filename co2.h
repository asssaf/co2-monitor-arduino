#include <SensirionI2CScd4x.h>

namespace co2 {
  enum State { INIT, MEASURING, DONE };

  struct Result {
    State state;
    int co2;
    float temperature;
    float humidity;
    int correction;
  };

  class Task {
  public:
    virtual ~Task() {}

    virtual Result run() = 0;
  };

  class FakeTask : public Task {
    Result run() override;

    Result result;
  };

  class SCD4xTask : public Task {
  public:
    SCD4xTask(bool calibration_mode) {
      this->calibration_mode = calibration_mode;
      this->next_millis = 0;
      this->result = Result{
        state: INIT
      };
    }

    Result run() override;

    Result result;

  private:
    SensirionI2CScd4x scd4x;
    int errorCount;
    unsigned long next_millis;
    bool calibration_mode;
  };
}
