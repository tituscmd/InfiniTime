#include "heartratetask/HeartRateTask.h"
#include <drivers/Hrs3300.h>
#include <components/heartrate/HeartRateController.h>
#include <nrf_log.h>

using namespace Pinetime::Applications;

TickType_t CurrentTaskDelay(HeartRateTask::States state, TickType_t ppgDeltaTms) {
  switch (state) {
    case HeartRateTask::States::Measuring:
    case HeartRateTask::States::BackgroundMeasuring:
      return ppgDeltaTms;
    case HeartRateTask::States::Running:
      return pdMS_TO_TICKS(100);
    case HeartRateTask::States::BackgroundWaiting:
      return pdMS_TO_TICKS(10000);
    default:
      return portMAX_DELAY;
  }
}


HeartRateTask::HeartRateTask(Drivers::Hrs3300& heartRateSensor,
                             Controllers::HeartRateController& controller,
                             Controllers::Settings& settings)
  : heartRateSensor {heartRateSensor}, controller {controller}, settings {settings} {
}

void HeartRateTask::Start() {
  messageQueue = xQueueCreate(10, 1);
  controller.SetHeartRateTask(this);

  if (pdPASS != xTaskCreate(HeartRateTask::Process, "Heartrate", 500, this, 0, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void HeartRateTask::Process(void* instance) {
  auto* app = static_cast<HeartRateTask*>(instance);
  app->Work();
}

void HeartRateTask::Work() {
  int lastBpm = 0;

  while (true) {
    TickType_t delay = CurrentTaskDelay(state, ppg.deltaTms);
    Messages msg;

    if (xQueueReceive(messageQueue, &msg, delay) == pdTRUE) {
      switch (msg) {
        case Messages::GoToSleep:
          HandleGoToSleep();
          break;
        case Messages::WakeUp:
          HandleWakeUp();
          break;
        case Messages::StartMeasurement:
          HandleStartMeasurement(&lastBpm);
          break;
        case Messages::StopMeasurement:
          HandleStopMeasurement();
          break;
      }
    }

    if (measurementStarted) {
      auto sensorData = heartRateSensor.ReadHrsAls();
      int8_t ambient = ppg.Preprocess(sensorData.hrs, sensorData.als);
      int bpm = ppg.HeartRate();

      // If ambient light detected or a reset requested (bpm < 0)
      if (ambient > 0) {
        // Reset all DAQ buffers
        ppg.Reset(true);
        // Force state to NotEnoughData (below)
        lastBpm = 0;
        bpm = 0;
      } else if (bpm < 0) {
        // Reset all DAQ buffers except HRS buffer
        ppg.Reset(false);
        // Set HR to zero and update
        bpm = 0;
        controller.Update(Controllers::HeartRateController::States::Running, bpm);
      }

      if (lastBpm == 0 && bpm == 0) {
        controller.Update(Controllers::HeartRateController::States::NotEnoughData, bpm);
      }

      if (bpm != 0) {
        lastBpm = bpm;
        controller.Update(Controllers::HeartRateController::States::Running, lastBpm);
      }
    }
  }
}

void HeartRateTask::PushMessage(HeartRateTask::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(messageQueue, &msg, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HeartRateTask::StartMeasurement() {
  heartRateSensor.Enable();
  ppg.Reset(true);
  vTaskDelay(100);
  measurementStart = xTaskGetTickCount();
}

void HeartRateTask::StopMeasurement() {
  heartRateSensor.Disable();
  ppg.Reset(true);
  vTaskDelay(100);
}

void HeartRateTask::StartWaiting() {
  StopMeasurement();
  backgroundWaitingStart = xTaskGetTickCount();
}

void HeartRateTask::HandleGoToSleep() {
  switch (state) {
    case States::Running:
      state = States::Idle;
      break;
    case States::Measuring:
      state = States::BackgroundMeasuring;
      break;
    case States::Idle:
    case States::BackgroundWaiting:
    case States::BackgroundMeasuring:
      // shouldn't happen -> ignore
      break;
  }
}

void HeartRateTask::HandleWakeUp() {
  switch (state) {
    case States::Idle:
      state = States::Running;
      break;
    case States::BackgroundMeasuring:
      state = States::Measuring;
      break;
    case States::BackgroundWaiting:
      state = States::Measuring;
      StartMeasurement();
      break;
    case States::Running:
    case States::Measuring:
      // shouldn't happen -> ignore
      break;
  }
}

void HeartRateTask::HandleStartMeasurement(int* lastBpm) {
  switch (state) {
    case States::Idle:
    case States::Running:
      state = States::Measuring;
      *lastBpm = 0;
      StartMeasurement();
      break;
    case States::Measuring:
    case States::BackgroundMeasuring:
    case States::BackgroundWaiting:
      // shouldn't happen -> ignore
      break;
  }
}

void HeartRateTask::HandleStopMeasurement() {
  switch (state) {
    case States::Measuring:
      state = States::Running;
      StopMeasurement();
      break;
    case States::BackgroundMeasuring:
    case States::BackgroundWaiting:
      state = States::Idle;
      StopMeasurement();
      break;
    case States::Running:
    case States::Idle:
      // shouldn't happen -> ignore
      break;
  }
}

void HeartRateTask::HandleBackgroundWaiting() {
  if (!IsBackgroundMeasurementActivated()) {
    return;
  }

  TickType_t ticksSinceWaitingStart = xTaskGetTickCount() - backgroundWaitingStart;
  if (ticksSinceWaitingStart >= GetHeartRateBackgroundMeasurementIntervalInTicks()) {
    state = States::BackgroundMeasuring;
    StartMeasurement();
  }
}

void HeartRateTask::HandleSensorData(int* lastBpm) {
  int8_t ambient = ppg.Preprocess(heartRateSensor.ReadHrs(), heartRateSensor.ReadAls());
  int bpm = ppg.HeartRate();

  // If ambient light detected or a reset requested (bpm < 0)
  if (ambient > 0) {
    // Reset all DAQ buffers
    ppg.Reset(true);
  } else if (bpm < 0) {
    // Reset all DAQ buffers except HRS buffer
    ppg.Reset(false);
    // Set HR to zero and update
    bpm = 0;
  }

  if (*lastBpm == 0 && bpm == 0) {
    controller.Update(Controllers::HeartRateController::States::NotEnoughData, bpm);
  }

  if (bpm != 0) {
    *lastBpm = bpm;
    controller.Update(Controllers::HeartRateController::States::Running, bpm);
    if (state == States::Measuring || IsContinuosModeActivated()) {
      return;
    }
    if (state == States::BackgroundMeasuring) {
      state = States::BackgroundWaiting;
      StartWaiting();
    }
  }
  TickType_t ticksSinceMeasurementStart = xTaskGetTickCount() - measurementStart;
  if (bpm == 0 && state == States::BackgroundMeasuring && !IsContinuosModeActivated() &&
      ticksSinceMeasurementStart >= DURATION_UNTIL_BACKGROUND_MEASUREMENT_IS_STOPPED) {
    state = States::BackgroundWaiting;
    StartWaiting();
  }
}

TickType_t HeartRateTask::GetHeartRateBackgroundMeasurementIntervalInTicks() {
  int ms;
  switch (settings.GetHeartRateBackgroundMeasurementInterval()) {
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::TenSeconds:
      ms = 10 * 1000;
      break;
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::ThirtySeconds:
      ms = 30 * 1000;
      break;
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::OneMinute:
      ms = 60 * 1000;
      break;
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::FiveMinutes:
      ms = 5 * 60 * 1000;
      break;
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::TenMinutes:
      ms = 10 * 60 * 1000;
      break;
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::ThirtyMinutes:
      ms = 30 * 60 * 1000;
      break;
    default:
      ms = 0;
      break;
  }
  return pdMS_TO_TICKS(ms);
}

bool HeartRateTask::IsContinuosModeActivated() {
  return settings.GetHeartRateBackgroundMeasurementInterval() ==
         Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::Continuous;
}

bool HeartRateTask::IsBackgroundMeasurementActivated() {
  return settings.GetHeartRateBackgroundMeasurementInterval() !=
         Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::Off;
}
