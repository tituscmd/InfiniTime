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
      static void StopMotor(TimerHandle_t xTimer);
      static void NotifBuzz(TimerHandle_t xTimer);
      /*
      void StartRinging();
      void StopRinging();
      */

    private:
      static void AlarmRing(TimerHandle_t xTimer);
      static void CallRing(TimerHandle_t xTimer);
      static void TimerRing(TimerHandle_t xTimer);
      static void ChimeBuzz(TimerHandle_t xTimer);
      
      TimerHandle_t alarmVib;
      TimerHandle_t callVib;
      TimerHandle_t timerVib;
      TimerHandle_t chimeVib;
      TimerHandle_t notifVib;
    };
  }
}
