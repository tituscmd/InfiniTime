#include "displayapp/screens/HeartRate.h"
#include <lvgl/lvgl.h>
#include <components/heartrate/HeartRateController.h>

#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  const char* ToString(Pinetime::Controllers::HeartRateController::States s) {
    switch (s) {
      case Pinetime::Controllers::HeartRateController::States::NotEnoughData:
        return "Hold still...";
      case Pinetime::Controllers::HeartRateController::States::NoTouch:
        return "No touch detected";
      case Pinetime::Controllers::HeartRateController::States::Running:
        return "Measuring...";
      case Pinetime::Controllers::HeartRateController::States::Stopped:
        return "Stopped";
    }
    return "";
  }

  void btnStartStopEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<HeartRate*>(obj->user_data);
    screen->OnStartStopEvent(event);
  }
}

HeartRate::HeartRate(Controllers::HeartRateController& heartRateController, System::SystemTask& systemTask)
  : heartRateController {heartRateController}, wakeLock(systemTask) {
  bool isHrRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;

  label_hr_value = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_hr_value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(label_hr_value, "--");
  lv_obj_align(label_hr_value, nullptr, LV_ALIGN_CENTER, 0, 10);
  lv_obj_set_auto_realign(label_hr_value, true);

  label_hr_text = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_hr_text, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_text_static(label_hr_text, "BPM");
  lv_obj_align(label_hr_text, label_hr_value, LV_ALIGN_CENTER, 43, 9);

  if (isHrRunning) {
    lv_obj_set_style_local_text_color(label_hr_value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
  } else {
    lv_obj_set_style_local_text_color(label_hr_value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  }

  label_bpm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_bpm, "Heart rate BPM");
  lv_obj_align(label_bpm, nullptr, LV_ALIGN_OUT_TOP_MID, 0, 30);

  label_status = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_status, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text_static(label_status, ToString(Pinetime::Controllers::HeartRateController::States::NotEnoughData));

  heart = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(heart, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &hrm_icon);
  lv_label_set_text_static(heart, Symbols::heart);
  lv_obj_set_style_local_text_color(heart, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_align(heart, nullptr, LV_ALIGN_CENTER, 0, -40);

  btn_startStop = lv_btn_create(lv_scr_act(), nullptr);
  btn_startStop->user_data = this;
  lv_obj_set_height(btn_startStop, 50);
  lv_obj_set_event_cb(btn_startStop, btnStartStopEventHandler);
  lv_obj_align(btn_startStop, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  lv_obj_align(label_status, btn_startStop, LV_ALIGN_OUT_BOTTOM_MID, 0, -20);

  label_startStop = lv_label_create(btn_startStop, nullptr);
  UpdateStartStopButton(isHrRunning);
  if (isHrRunning) {
    wakeLock.Lock();
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, 100, LV_TASK_PRIO_MID, this);
}

HeartRate::~HeartRate() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void HeartRate::Refresh() {

  auto state = heartRateController.State();
  switch (state) {
    case Controllers::HeartRateController::States::NoTouch:
    case Controllers::HeartRateController::States::NotEnoughData:
      // case Controllers::HeartRateController::States::Stopped:
      lv_label_set_text_static(label_hr_value, "--");
      lv_obj_align(label_hr_text, label_hr_value, LV_ALIGN_CENTER, 43, 9);
      break;
    default:
      if (heartRateController.HeartRate() == 0) {
        lv_label_set_text_static(label_hr_value, "111"); // revert to "--" when done, this is just for UI testing
      } else {
        lv_label_set_text_fmt(label_hr_value, "%d", heartRateController.HeartRate());
        if(heartRateController.HeartRate() > 99) {
          lv_obj_align(label_hr_text, label_hr_value, LV_ALIGN_CENTER, 56, 9);
        }
        else {
          lv_obj_align(label_hr_text, label_hr_value, LV_ALIGN_CENTER, 43, 9); // 43
        }
      }
    }

  lv_label_set_text_static(label_status, ToString(state));
  lv_obj_align(label_status, btn_startStop, LV_ALIGN_OUT_BOTTOM_MID, 0, -20);
}

void HeartRate::OnStartStopEvent(lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (heartRateController.State() == Controllers::HeartRateController::States::Stopped) {
      heartRateController.Start();
      UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
      wakeLock.Lock();
      lv_obj_set_style_local_text_color(label_hr_value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
    } else {
      heartRateController.Stop();
      UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
      wakeLock.Release();
      lv_obj_set_style_local_text_color(label_hr_value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
    }
  }
}

void HeartRate::UpdateStartStopButton(bool isRunning) {
  if (isRunning) {
    lv_label_set_text_static(label_startStop, "Stop");
  } else {
    lv_label_set_text_static(label_startStop, "Start");
  }
}
