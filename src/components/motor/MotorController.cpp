#include "components/motor/MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "drivers/PinMap.h"
#include "nrf_pwm.h"

using namespace Pinetime::Controllers;

static uint16_t pwmValue = 0; // Declare the variable for PWM value

void MotorController::Init() {
  // Configure the motor pin as an output
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  // Configure the PWM sequence
  static nrf_pwm_sequence_t seq;
  seq.values.p_common = &pwmValue; // Use the PWM value array
  seq.length = NRF_PWM_VALUES_LENGTH(pwmValue);
  seq.repeats = 0;
  seq.end_delay = 0;

  // Configure the PWM pins
  uint32_t out_pins[] = {PinMap::Motor, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED};
  nrf_pwm_pins_set(NRF_PWM2, out_pins);

  // Enable and configure the PWM peripheral
  nrf_pwm_enable(NRF_PWM2);
  nrf_pwm_configure(NRF_PWM2, NRF_PWM_CLK_1MHz, NRF_PWM_MODE_UP, 255); // Top value determines the resolution
  nrf_pwm_loop_set(NRF_PWM2, 0);                                       // Infinite loop
  nrf_pwm_decoder_set(NRF_PWM2, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);
  nrf_pwm_sequence_set(NRF_PWM2, 0, &seq);

  // Start the PWM with an initial value of 0
  pwmValue = 0;
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_SEQSTART0);

  // Initialize timers for motor actions
  notifVib = xTimerCreate("notifVib", 1, pdFALSE, nullptr, StopMotor);
  alarmVib1 = xTimerCreate("alarmVib", pdMS_TO_TICKS(1000), pdTRUE, this, AlarmRing1);
  alarmVib2 = xTimerCreate("alarmVib2", pdMS_TO_TICKS(100), pdFALSE, this, AlarmRing2);
  alarmVib3 = xTimerCreate("alarmVib3", pdMS_TO_TICKS(100), pdFALSE, this, AlarmRing3);
  callVib1 = xTimerCreate("callVib", pdMS_TO_TICKS(1000), pdTRUE, this, CallRing1);
  callVib2 = xTimerCreate("callVib2", pdMS_TO_TICKS(150), pdFALSE, this, CallRing2);
  timerVib1 = xTimerCreate("timerVib", pdMS_TO_TICKS(1000), pdTRUE, this, TimerRing1);
  timerVib2 = xTimerCreate("timerVib2", pdMS_TO_TICKS(100), pdFALSE, this, TimerRing2);
  timerVib3 = xTimerCreate("timerVib3", pdMS_TO_TICKS(100), pdFALSE, this, TimerRing3);
  timerVib4 = xTimerCreate("timerVib4", pdMS_TO_TICKS(100), pdFALSE, this, TimerRing4);
  chimeVib1 = xTimerCreate("chimeVib1", 1, pdFALSE, this, ChimeRing1);
  chimeVib2 = xTimerCreate("chimeVib2", pdMS_TO_TICKS(100), pdFALSE, this, ChimeRing2);
  wakeAlarmVib = xTimerCreate("wakeAlarmVib", pdMS_TO_TICKS(1000), pdTRUE, this, WakeAlarmRing);
  naturalWakeAlarmVib = xTimerCreate("natWakeVib", pdMS_TO_TICKS(30 * 1000), pdTRUE, this, NaturalWakeAlarmRing);
  chimeVib1 = xTimerCreate("chimeVib1", 1, pdFALSE, this, ChimeRing1);
  chimeVib2 = xTimerCreate("chimeVib2", pdMS_TO_TICKS(100), pdFALSE, this, ChimeRing2);
}

void MotorController::SetMotorStrength(uint8_t strength) {
  // Ensure strength is within bounds (0-100)
  // if (strength > 100)
  //   strength = 100;

  // Map the strength to the PWM value (0-100 -> 0-top_value)
  // pwmValue = (strength * 255) / 100;
  pwmValue = strength;
}

void MotorController::StartWakeAlarm() {
  wakeAlarmStrength = (80 * infiniSleepMotorStrength) / 100;
  wakeAlarmDuration = 100;
  SetMotorStrength(wakeAlarmStrength);
  RunForDuration(wakeAlarmDuration);
  xTimerStart(wakeAlarmVib, 0);
}

void MotorController::WakeAlarmRing(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  if (motorController->wakeAlarmStrength > (40 * motorController->infiniSleepMotorStrength) / 100) {
    motorController->wakeAlarmStrength -= (1 * motorController->infiniSleepMotorStrength) / 100;
  }
  if (motorController->wakeAlarmDuration < 500) {
    motorController->wakeAlarmDuration += 6;
  }
  motorController->SetMotorStrength(motorController->wakeAlarmStrength);
  motorController->RunForDuration(motorController->wakeAlarmDuration);
}

void MotorController::StopWakeAlarm() {
  xTimerStop(wakeAlarmVib, 0);
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_STOP); // Stop the PWM sequence
  pwmValue = 0;                                      // Reset the PWM value
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StartNaturalWakeAlarm() {
  wakeAlarmStrength = (80 * infiniSleepMotorStrength) / 100;
  wakeAlarmDuration = 100;
  SetMotorStrength(wakeAlarmStrength);
  RunForDuration(wakeAlarmDuration);
  xTimerStart(naturalWakeAlarmVib, 0);
}

void MotorController::NaturalWakeAlarmRing(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  if (motorController->wakeAlarmStrength > (80 * motorController->infiniSleepMotorStrength) / 100) {
    motorController->wakeAlarmStrength -= (5 * motorController->infiniSleepMotorStrength) / 100;
  } else {
      motorController->wakeAlarmStrength += (10 * motorController->infiniSleepMotorStrength) / 100;
  }

  if (motorController->wakeAlarmDuration <= 100) {
      motorController->wakeAlarmDuration += 25;
  } else {
      motorController->wakeAlarmDuration -= 50;
  }
  motorController->SetMotorStrength(motorController->wakeAlarmStrength);
  motorController->RunForDuration(motorController->wakeAlarmDuration);
}

void MotorController::StopNaturalWakeAlarm() {
  xTimerStop(naturalWakeAlarmVib, 0);
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_STOP); // Stop the PWM sequence
  pwmValue = 0;                                      // Reset the PWM value
  nrf_gpio_pin_set(PinMap::Motor);
}

// 2x 25ms buzz + 75ms buzz every 1 second
void MotorController::AlarmRing1(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(15);
  xTimerStart(motorController->alarmVib2, 0);
}

void MotorController::AlarmRing2(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(15);
  xTimerStart(motorController->alarmVib3, 0);
}

void MotorController::AlarmRing3(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(35);
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

void MotorController::RunForDuration(uint16_t motorDuration) {
  if (motorDuration > 0 && xTimerChangePeriod(notifVib, pdMS_TO_TICKS(motorDuration), 0) == pdPASS && xTimerStart(notifVib, 0) == pdPASS) {
    if (pwmValue == 0) {
      SetMotorStrength(255);
    }
    // nrf_gpio_pin_clear(PinMap::Motor);
    nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_SEQSTART0); // Restart the PWM sequence with the updated value
  }
}

// public starting methods
void MotorController::StartAlarmRing() {
  xTimerStart(alarmVib1, 0);
}

void MotorController::StopAlarmRing() {
  xTimerStop(alarmVib1, 0);
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StartCallRing() {
  xTimerStart(callVib1, 0);
}

void MotorController::StopCallRing() {
  xTimerStop(callVib1, 0);
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StartChimeRing() {
  xTimerStart(chimeVib1, 0);
}

void MotorController::StartTimerRing() {
  xTimerStart(timerVib1, 0);
}

void MotorController::StopTimerRing() {
  xTimerStop(timerVib1, 0);
  nrf_gpio_pin_set(PinMap::Motor);
}

bool MotorController::IsRinging() {
  return (xTimerIsTimerActive(timerVib1) == pdTRUE);
}

// just stops the motor
void MotorController::StopMotor(TimerHandle_t /*xTimer*/) {
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_STOP); // Stop the PWM sequence
  pwmValue = 0;                                      // Reset the PWM value
  nrf_gpio_pin_set(PinMap::Motor);                   // Set the motor pin to the off state
}

// infinisleep stuff
void MotorController::GradualWakeBuzz() {
  SetMotorStrength((60 * infiniSleepMotorStrength) / 100);
  RunForDuration(100);
}
