#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {

    class MotorController {
    public:
      MotorController() = default;

      void Init();
      void RunForDuration(uint8_t motorDuration);
      void StartAlarmRing();
      void StopAlarmRing();
      void StartCallRing();
      void StopCallRing();
      void NotifBuzz();
      void StartTimerRing();
      void StopTimerRing();

    private:
      static void StopMotor(TimerHandle_t xTimer);
      static void AlarmRing1(TimerHandle_t xTimer);
      static void AlarmRing2(TimerHandle_t xTimer);
      static void AlarmRing3(TimerHandle_t xTimer);
      static void CallRing1(TimerHandle_t xTimer);
      static void CallRing2(TimerHandle_t xTimer);
      static void TimerRing1(TimerHandle_t xTimer);
      static void TimerRing2(TimerHandle_t xTimer);
      static void TimerRing3(TimerHandle_t xTimer);
      static void TimerRing4(TimerHandle_t xTimer);
      TimerHandle_t alarmVib;
      TimerHandle_t alarmVib2;
      TimerHandle_t alarmVib3;
      TimerHandle_t callVib;
      TimerHandle_t callVib2;
      TimerHandle_t timerVib;
      TimerHandle_t timerVib2;
      TimerHandle_t timerVib3;
      TimerHandle_t timerVib4;
      TimerHandle_t notifVib;
    };
  }
}
