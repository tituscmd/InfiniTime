#include "components/motor/MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "drivers/PinMap.h"

using namespace Pinetime::Controllers;

void MotorController::Init() {
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  notifVib = xTimerCreate("notifVib", 1, pdFALSE, nullptr, StopMotor);
  alarmVib = xTimerCreate("alarmVib", pdMS_TO_TICKS(1000), pdTRUE, this, AlarmRing1);
  alarmVib2 = xTimerCreate("alarmVib2", pdMS_TO_TICKS(100), pdFALSE, this, AlarmRing2);
  alarmVib3 = xTimerCreate("alarmVib3", pdMS_TO_TICKS(100), pdFALSE, this, AlarmRing3);
  callVib = xTimerCreate("callVib", pdMS_TO_TICKS(1000), pdTRUE, this, CallRing1);
  callVib2 = xTimerCreate("callVib2", pdMS_TO_TICKS(150), pdFALSE, this, CallRing2);
  timerVib = xTimerCreate("timerVib", pdMS_TO_TICKS(1000), pdTRUE, this, TimerRing1);
  timerVib2 = xTimerCreate("timerVib2", pdMS_TO_TICKS(100), pdFALSE, this, TimerRing2);
  timerVib3 = xTimerCreate("timerVib3", pdMS_TO_TICKS(100), pdFALSE, this, TimerRing3);
  timerVib4 = xTimerCreate("timerVib4", pdMS_TO_TICKS(100), pdFALSE, this, TimerRing4);
  chimeVib1 = xTimerCreate("chimeVib1", 1, pdFALSE, this, ChimeRing1);
  chimeVib2 = xTimerCreate("chimeVib2", pdMS_TO_TICKS(100), pdFALSE, this, ChimeRing2);
}

// 2x 25ms buzz + 75ms buzz every 1 second
void MotorController::AlarmRing1(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(25);
  xTimerStart(motorController->alarmVib2, 0);
}

void MotorController::AlarmRing2(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(25);
  xTimerStart(motorController->alarmVib3, 0);
}

void MotorController::AlarmRing3(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(75);
}

// 50ms double buzz with 150ms gap, every 1 second
void MotorController::CallRing1(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
  xTimerStart(motorController->callVib2, 0);
}

void MotorController::CallRing2(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

// 25ms buzz + 10ms + 10ms + 10ms
void MotorController::TimerRing1(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(25);
  xTimerStart(motorController->timerVib2, 0);
}

void MotorController::TimerRing2(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(10);
  xTimerStart(motorController->timerVib3, 0);
}

void MotorController::TimerRing3(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(10);
  xTimerStart(motorController->timerVib4, 0);
}

void MotorController::TimerRing4(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(10);
}

void MotorController::ChimeRing1(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(20);
  xTimerStart(motorController->chimeVib2, 0);
}

void MotorController::ChimeRing2(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(20);
}

// single buzz, very nice on the wrist
void MotorController::NotifBuzz() {
  RunForDuration(30);
}

// single buzz depending on motorDuration
void MotorController::RunForDuration(uint8_t motorDuration) {
  if (motorDuration > 0 && xTimerChangePeriod(notifVib, pdMS_TO_TICKS(motorDuration), 0) == pdPASS && xTimerStart(notifVib, 0) == pdPASS) {
    nrf_gpio_pin_clear(PinMap::Motor);
  }
}

void MotorController::StartAlarmRing() {
  xTimerStart(alarmVib, 0);
}

void MotorController::StopAlarmRing() {
  xTimerStop(alarmVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StartCallRing() {
  xTimerStart(callVib, 0);
}

void MotorController::StopCallRing() {
  xTimerStop(callVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StartChimeRing() {
  xTimerStart(chimeVib1, 0);
}

void MotorController::StopMotor(TimerHandle_t /*xTimer*/) {
  nrf_gpio_pin_set(PinMap::Motor);
}
