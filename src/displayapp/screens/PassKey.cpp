#include "PassKey.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

PassKey::PassKey(uint32_t key) {
  passkeyValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(passkeyValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_set_style_local_text_font(passkeyValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(passkeyValue, "%06u", key);
  lv_obj_align(passkeyValue, nullptr, LV_ALIGN_CENTER, 0, -20);
}

PassKey::~PassKey() {
  lv_obj_clean(lv_scr_act());
}
