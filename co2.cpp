#include "co2.h"
#include <SensirionI2CScd4x.h>

#define FIRST_MEASUREMENT_MS 4500
#define CLIBRATION_TIME_MS 180000  // recommended minimum 3 minutes of measurement before calibration
#define CALIBRATION_CO2_TARGET 400 // fresh air is around 400

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

    if (millis() < this->next_millis) {
      return this->result;
    }

    switch (this->result.state) {
    case INIT:
      scd4x.begin(Wire);
      error = scd4x.stopPeriodicMeasurement();
      if (error) {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        errorCount++;
        if (errorCount > 20) {
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
        if (errorCount > 20) {
          this->result.state = DONE;
        }
        break;
      }

      errorCount = 0;
      if (calibration_mode) {
        this->next_millis = millis() + CLIBRATION_TIME_MS;
      } else {
        this->next_millis = millis() + FIRST_MEASUREMENT_MS;
      }
      this->result.state = MEASURING;
      break;

    case MEASURING:
      error = scd4x.readMeasurement(co2, temperature, humidity);
      if (error) {
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        errorCount++;
        if (errorCount > 10) {
          this->result.state = DONE;
        } else {
          this->next_millis = millis() + 1000;
        }
        break;

      } else if (co2 == 0) {
        Serial.println("Invalid sample detected, skipping.");
        this->next_millis = millis() + 1000;
        break;
      }

      this->result.state = DONE;
      this->result.co2 = co2;
      this->result.temperature = temperature;
      this->result.humidity = humidity;

      error = scd4x.stopPeriodicMeasurement();
      if (error) {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
      }

      if (calibration_mode) {
        uint16_t correction;
        Serial.println("Performing forced calibration to " + String(CALIBRATION_CO2_TARGET) + ", measurement was " + String(co2));
        error = scd4x.performForcedRecalibration(CALIBRATION_CO2_TARGET, correction);
        if (error) {
          Serial.print("Error trying to execute performForcedRecalibration(): ");
          errorToString(error, errorMessage, 256);
          Serial.println(errorMessage);
        } else if (correction = 0xFFFF) {
          Serial.print("Calibration failed: ");
          errorToString(error, errorMessage, 256);
          Serial.println(errorMessage);
        } else {
          Serial.println("calibration correction: " + String(correction));

          this->result.correction = correction - 0x8000;
        }
      }
      break;

    case DONE:
      break;
    }

    return result;
  }

}
