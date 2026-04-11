#include "ui_main.h"
#include "lvgl.h"

void ui_main_init(void) {
  lv_obj_t *screen = lv_screen_active();

  /* Set screen background to black */
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  /* Create label */
  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, "Hello LVGL");

  /* Set text color to white */
  lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);

  /* Center the label */
  lv_obj_center(label);
}