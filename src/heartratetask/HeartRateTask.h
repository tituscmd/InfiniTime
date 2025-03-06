#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <components/heartrate/Ppg.h>
#include "components/settings/Settings.h"

#define DURATION_UNTIL_BACKGROUND_MEASUREMENT_IS_STOPPED pdMS_TO_TICKS(30 * 1000)

namespace Pinetime {
  namespace Drivers {
    class Hrs3300;
  }

  namespace Controllers {
    class HeartRateController;
  }

  namespace Applications {
    class HeartRateTask {
    public:
      enum class Messages : uint8_t { GoToSleep, WakeUp, StartMeasurement, StopMeasurement };

      explicit HeartRateTask(Drivers::Hrs3300& heartRateSensor,
                             Controllers::HeartRateController& controller,
                             Controllers::Settings& settings);
      void Start();
      void Work();
      void PushMessage(Messages msg);

    private:
      static void Process(void* instance);
      void StartSensor();
      void StopSensor();

      void HandleGoToSleep();
      void HandleWakeUp();
      void HandleStartMeasurement(int* lastBpm);
      void HandleStopMeasurement();

      void HandleWaiting();
      void HandleSensorData(int* lastBpm);

      TickType_t GetBackgroundIntervalInTicks();
      bool IsContinuousModeActivated();
      bool IsBackgroundMeasurementActivated();

      TickType_t GetTicksSinceLastMeasurementStarted();
      bool ShoudStopTryingToGetData();
      bool ShouldStartBackgroundMeasuring();

      TaskHandle_t taskHandle;
      QueueHandle_t messageQueue;

      bool isBackgroundMeasuring = false;
      bool isScreenOn = true;
      bool isMeasurementActivated = false;

      Drivers::Hrs3300& heartRateSensor;
      Controllers::HeartRateController& controller;
      Controllers::Settings& settings;
      Controllers::Ppg ppg;

      TickType_t measurementStart = 0;
    };

  }
}
