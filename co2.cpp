#include "co2.h"
#include <SensirionI2CScd4x.h>

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

  Result SCD4xTask::run() {
    uint16_t error;
    char errorMessage[256];
    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;

    switch (this->result.state) {
    case INIT:
      scd4x.begin(Wire);
      error = scd4x.stopPeriodicMeasurement();
      if (error) {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        errorCount++;
        if (errorCount > 5) {
          this->result.state = DONE;
        }
        break;
      }

      error = scd4x.startPeriodicMeasurement();
      if (error) {
        Serial.print("Error trying to execute startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        errorCount++;
        if (errorCount > 5) {
          this->result.state = DONE;
        }
        break;
      }

      errorCount = 0;
      this->result.state = MEASURING;
      break;

    case MEASURING:
      error = scd4x.readMeasurement(co2, temperature, humidity);
      if (error) {
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        errorCount++;
        if (errorCount > 5) {
          this->result.state = DONE;
        }
        break;

      } else if (co2 == 0) {
        Serial.println("Invalid sample detected, skipping.");
        break;
      }

      this->result.state = DONE;
      this->result.co2 = co2;

      error = scd4x.stopPeriodicMeasurement();
      if (error) {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
      }

      break;

    case DONE:
      break;
    }

    return result;
  }

}
