#include "components/motor/MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "drivers/PinMap.h"

using namespace Pinetime::Controllers;

void MotorController::Init() {
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  alarmVib = xTimerCreate("alarmVib", pdMS_TO_TICKS(1000), pdTRUE, this, AlarmRing);
  callVib = xTimerCreate("callVib", pdMS_TO_TICKS(1000), pdTRUE, this, CallRing);
  timerVib = xTimerCreate("timerVib", pdMS_TO_TICKS(1000), pdTRUE, this, TimerRing);
  chimeVib = xTimerCreate("chimeVib", 1, pdFALSE, nullptr, StopMotor);
  notifVib = xTimerCreate("notifVib", 1, pdFALSE, nullptr, StopMotor);
}

// 50ms buzz every 1 second
void MotorController::AlarmRing(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

// 50ms double buzz with 150ms gap, every 1 second
void MotorController::CallRing(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

// 25ms triple buzz with 50ms gap, every 1 second
void MotorController::TimerRing(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

// 25ms double buzz with 100ms gap
void MotorController::ChimeBuzz(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

// singe 50ms buzz
void MotorController::NotifBuzz(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

// single buzz depending on motorDuration
void MotorController::RunForDuration(uint8_t motorDuration) {
  if (motorDuration > 0 && xTimerChangePeriod(shortVib, pdMS_TO_TICKS(motorDuration), 0) == pdPASS && xTimerStart(shortVib, 0) == pdPASS) {
    nrf_gpio_pin_clear(PinMap::Motor);
  }
}

/*
void MotorController::StartRinging() {
  RunForDuration(50);
  xTimerStart(longVib, 0);
}

void MotorController::StopRinging() {
  xTimerStop(longVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
}
*/

void MotorController::StopMotor(TimerHandle_t /*xTimer*/) {
  nrf_gpio_pin_set(PinMap::Motor);
}
