#include "ui_main.h"
#include "album.h"
#include "lvgl.h"
#include "photo_view.h"
#include "thumb_view.h"
#include <stdio.h>

static lv_obj_t *mainScreen;
static lv_obj_t *thumbScreen;

static lv_obj_t *mainScreen;
static lv_obj_t *thumbScreen;
static lv_obj_t *photoImg;

static lv_obj_t *thumbImgs[4];
static uint32_t gThumbPageStart = 0;

extern uint32_t gCurrentPhotoIndex;

static void PrevBtnEvent(lv_event_t *e) {
  (void)e;
  printf("Prev pressed\r\n");
  Album_ShowPrev();
}

static void NextBtnEvent(lv_event_t *e) {
  (void)e;
  printf("Next pressed\r\n");
  Album_ShowNext();
}

static void ReloadThumbPage(void) {
  uint32_t count = Album_GetCount();

  Album_LoadThumbPage(gThumbPageStart);

  if (gThumbPageStart + 0U < count)
    ThumbView_SetThumb(0, THUMB_BUF0, THUMB_W, THUMB_H, gThumbPageStart + 0U);
  if (gThumbPageStart + 1U < count)
    ThumbView_SetThumb(1, THUMB_BUF1, THUMB_W, THUMB_H, gThumbPageStart + 1U);
  if (gThumbPageStart + 2U < count)
    ThumbView_SetThumb(2, THUMB_BUF2, THUMB_W, THUMB_H, gThumbPageStart + 2U);
  if (gThumbPageStart + 3U < count)
    ThumbView_SetThumb(3, THUMB_BUF3, THUMB_W, THUMB_H, gThumbPageStart + 3U);
}

static void ThumbBtnEvent(lv_event_t *e) {
  (void)e;
  gThumbPageStart = 0;
  ReloadThumbPage();
  lv_screen_load(thumbScreen);
}

static void Thumb0Event(lv_event_t *e) {
  uint32_t index;
  (void)e;

  if (ThumbView_GetPhotoIndex(0, &index) == 0) {
    Album_OpenFromThumb(index);
    lv_screen_load(mainScreen);
  }
}

static void Thumb1Event(lv_event_t *e) {
  uint32_t index;
  (void)e;

  if (ThumbView_GetPhotoIndex(1, &index) == 0) {
    Album_OpenFromThumb(index);
    lv_screen_load(mainScreen);
  }
}

static void Thumb2Event(lv_event_t *e) {
  uint32_t index;
  (void)e;

  if (ThumbView_GetPhotoIndex(2, &index) == 0) {
    Album_OpenFromThumb(index);
    lv_screen_load(mainScreen);
  }
}

static void Thumb3Event(lv_event_t *e) {
  uint32_t index;
  (void)e;

  if (ThumbView_GetPhotoIndex(3, &index) == 0) {
    Album_OpenFromThumb(index);
    lv_screen_load(mainScreen);
  }
}

static void BackBtnEvent(lv_event_t *e) {
  (void)e;
  Album_ShowByIndex(gCurrentPhotoIndex);
  lv_screen_load(mainScreen);
}

static void CreateMainScreen(void) {
  mainScreen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(mainScreen, lv_color_hex(0x111111), 0);
  lv_obj_set_style_bg_opa(mainScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(mainScreen, 0, 0);

  /* Title bar */
  lv_obj_t *titleBar = lv_obj_create(mainScreen);
  lv_obj_set_size(titleBar, 480, 60);
  lv_obj_align(titleBar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_radius(titleBar, 0, 0);
  lv_obj_set_style_bg_color(titleBar, lv_color_hex(0x222222), 0);
  lv_obj_set_style_border_width(titleBar, 0, 0);
  lv_obj_set_style_pad_all(titleBar, 0, 0);

  lv_obj_t *title = lv_label_create(titleBar);
  lv_label_set_text(title, "Gallery");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_center(title);

  /* Photo panel */
  lv_obj_t *photoPanel = lv_obj_create(mainScreen);
  lv_obj_set_size(photoPanel, 440, 620);
  lv_obj_align(photoPanel, LV_ALIGN_TOP_MID, 0, 80);
  lv_obj_set_style_bg_color(photoPanel, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_color(photoPanel, lv_color_hex(0x666666), 0);
  lv_obj_set_style_border_width(photoPanel, 2, 0);
  lv_obj_set_style_radius(photoPanel, 8, 0);

  // lv_obj_t *photoText = lv_label_create(photoPanel);
  // lv_label_set_text(photoText, "Photo Area");
  // lv_obj_set_style_text_color(photoText, lv_color_hex(0xAAAAAA), 0);
  // lv_obj_center(photoText);
  photoImg = lv_image_create(photoPanel);
  lv_obj_center(photoImg);

  /* Prev button */
  lv_obj_t *prevBtn = lv_button_create(mainScreen);
  lv_obj_set_size(prevBtn, 100, 50);
  lv_obj_align(prevBtn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
  lv_obj_add_event_cb(prevBtn, PrevBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *prevLabel = lv_label_create(prevBtn);
  lv_label_set_text(prevLabel, "Prev");
  lv_obj_center(prevLabel);

  /* Next button */
  lv_obj_t *nextBtn = lv_button_create(mainScreen);
  lv_obj_set_size(nextBtn, 100, 50);
  lv_obj_align(nextBtn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
  lv_obj_add_event_cb(nextBtn, NextBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *nextLabel = lv_label_create(nextBtn);
  lv_label_set_text(nextLabel, "Next");
  lv_obj_center(nextLabel);

  /* Thumb button */
  lv_obj_t *thumbBtn = lv_button_create(mainScreen);
  lv_obj_set_size(thumbBtn, 120, 50);
  lv_obj_align(thumbBtn, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_add_event_cb(thumbBtn, ThumbBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *thumbLabel = lv_label_create(thumbBtn);
  lv_label_set_text(thumbLabel, "Thumb");
  lv_obj_center(thumbLabel);
}

static void CreateThumbScreen(void) {
  thumbScreen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(thumbScreen, lv_color_hex(0x111111), 0);
  lv_obj_set_style_bg_opa(thumbScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(thumbScreen, 0, 0);

  lv_obj_t *titleBar = lv_obj_create(thumbScreen);
  lv_obj_set_size(titleBar, 480, 60);
  lv_obj_align(titleBar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_radius(titleBar, 0, 0);
  lv_obj_set_style_bg_color(titleBar, lv_color_hex(0x222222), 0);
  lv_obj_set_style_border_width(titleBar, 0, 0);
  lv_obj_set_style_pad_all(titleBar, 0, 0);

  lv_obj_t *title = lv_label_create(titleBar);
  lv_label_set_text(title, "Thumbnails");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_center(title);

  /* Back button */
  lv_obj_t *backBtn = lv_button_create(thumbScreen);
  lv_obj_set_size(backBtn, 80, 40);
  lv_obj_align(backBtn, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_event_cb(backBtn, BackBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *backLabel = lv_label_create(backBtn);
  lv_label_set_text(backLabel, "Back");
  lv_obj_center(backLabel);

  /* Thumbnail placeholders */
  for (int i = 0; i < 4; i++) {
    lv_obj_t *thumbBox = lv_obj_create(thumbScreen);
    lv_obj_set_size(thumbBox, 180, 220);

    int row = i / 2;
    int col = i % 2;

    lv_obj_set_pos(thumbBox, 40 + col * 220, 100 + row * 260);
    lv_obj_set_style_bg_color(thumbBox, lv_color_hex(0x222222), 0);
    lv_obj_set_style_border_color(thumbBox, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(thumbBox, 2, 0);
    lv_obj_set_style_radius(thumbBox, 8, 0);

    thumbImgs[i] = lv_image_create(thumbBox);
    lv_obj_center(thumbImgs[i]);

    ThumbView_BindImageObject((uint32_t)i, thumbImgs[i]);

    if (i == 0)
      lv_obj_add_event_cb(thumbBox, Thumb0Event, LV_EVENT_CLICKED, NULL);
    if (i == 1)
      lv_obj_add_event_cb(thumbBox, Thumb1Event, LV_EVENT_CLICKED, NULL);
    if (i == 2)
      lv_obj_add_event_cb(thumbBox, Thumb2Event, LV_EVENT_CLICKED, NULL);
    if (i == 3)
      lv_obj_add_event_cb(thumbBox, Thumb3Event, LV_EVENT_CLICKED, NULL);
  }
}

void ui_main_init(void) {
  CreateMainScreen();
  CreateThumbScreen();
  PhotoView_BindImageObject(photoImg);
  lv_screen_load(mainScreen);
}