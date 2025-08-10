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
      void RunForDuration(uint16_t motorDuration);
      void NotifBuzz();
      void StartAlarmRing();
      void StopAlarmRing();
      void StartCallRing();
      void StopCallRing();
      void StartChimeRing();
      void StopChimeRing();
      void StartTimerRing();
      void StopTimerRing();

      void StartWakeAlarm();
      void StopWakeAlarm();
      void StartNaturalWakeAlarm();
      void StopNaturalWakeAlarm();
      void GradualWakeBuzz();
      void StopGradualWakeBuzz();
      void SetMotorStrength(uint8_t strength);

      uint8_t wakeAlarmStrength = 80;
      uint16_t wakeAlarmDuration = 100;
      uint8_t infiniSleepMotorStrength = 100;

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
      static void ChimeRing1(TimerHandle_t xTimer);
      static void ChimeRing2(TimerHandle_t xTimer);
      TimerHandle_t alarmVib;
      TimerHandle_t alarmVib2;
      TimerHandle_t alarmVib3;
      TimerHandle_t callVib;
      TimerHandle_t callVib2;
      TimerHandle_t timerVib;
      TimerHandle_t timerVib2;
      TimerHandle_t timerVib3;
      TimerHandle_t timerVib4;
      TimerHandle_t chimeVib1;
      TimerHandle_t chimeVib2;
      TimerHandle_t notifVib;

      // ANCS stuff
      TimerHandle_t wakeAlarmVib;
      TimerHandle_t naturalWakeAlarmVib;
      static void Ring(TimerHandle_t xTimer);
      static void WakeAlarmRing(TimerHandle_t xTimer);
      static void NaturalWakeAlarmRing(TimerHandle_t xTimer);
    };
  }
}
