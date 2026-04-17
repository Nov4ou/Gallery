#include "ui_main.h"
#include "album.h"
#include "lvgl.h"
#include "photo_view.h"
#include <stdint.h>
#include <stdio.h>

static lv_obj_t *mainScreen;
static lv_obj_t *categoryScreen;
static lv_obj_t *photoImg;

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

static void CategoryBtnEvent(lv_event_t *e) {
  (void)e;
  lv_screen_load(categoryScreen);
}

static void CategoryItemEvent(lv_event_t *e) {
  AlbumCategory_t category;
  uint32_t count;

  category = (AlbumCategory_t)(uintptr_t)lv_event_get_user_data(e);
  count = Album_SetCategory(category);

  if (count == 0U) {
    printf("Category %d has no photo\r\n", (int)category);
    return;
  }

  Album_ShowCurrent();
  lv_screen_load(mainScreen);
}

static void BackBtnEvent(lv_event_t *e) {
  (void)e;
  Album_ShowByIndex(gCurrentPhotoIndex);
  lv_screen_load(mainScreen);
}

static lv_obj_t *CreateButton(lv_obj_t *parent, const char *text, int16_t w,
                              int16_t h) {
  lv_obj_t *btn;
  lv_obj_t *label;

  btn = lv_button_create(parent);
  lv_obj_set_size(btn, w, h);
  lv_obj_set_style_radius(btn, 8, 0);

  label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_center(label);

  return btn;
}

static void CreateMainScreen(void) {
  mainScreen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(mainScreen, lv_color_hex(0x111111), 0);
  lv_obj_set_style_bg_opa(mainScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(mainScreen, 0, 0);

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

  lv_obj_t *photoPanel = lv_obj_create(mainScreen);
  lv_obj_set_size(photoPanel, 440, 620);
  lv_obj_align(photoPanel, LV_ALIGN_TOP_MID, 0, 80);
  lv_obj_set_style_bg_color(photoPanel, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_color(photoPanel, lv_color_hex(0x666666), 0);
  lv_obj_set_style_border_width(photoPanel, 2, 0);
  lv_obj_set_style_radius(photoPanel, 8, 0);

  photoImg = lv_image_create(photoPanel);
  lv_obj_center(photoImg);

  lv_obj_t *prevBtn = CreateButton(mainScreen, "Prev", 100, 50);
  lv_obj_align(prevBtn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
  lv_obj_add_event_cb(prevBtn, PrevBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *nextBtn = CreateButton(mainScreen, "Next", 100, 50);
  lv_obj_align(nextBtn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
  lv_obj_add_event_cb(nextBtn, NextBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *categoryBtn = CreateButton(mainScreen, "Category", 140, 50);
  lv_obj_align(categoryBtn, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_add_event_cb(categoryBtn, CategoryBtnEvent, LV_EVENT_CLICKED, NULL);
}

static void AddCategoryButton(const char *text, AlbumCategory_t category,
                              int16_t y) {
  lv_obj_t *btn;

  btn = CreateButton(categoryScreen, text, 260, 70);
  lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y);
  lv_obj_add_event_cb(btn, CategoryItemEvent, LV_EVENT_CLICKED,
                      (void *)(uintptr_t)category);
}

static void CreateCategoryScreen(void) {
  categoryScreen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(categoryScreen, lv_color_hex(0x111111), 0);
  lv_obj_set_style_bg_opa(categoryScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(categoryScreen, 0, 0);

  lv_obj_t *titleBar = lv_obj_create(categoryScreen);
  lv_obj_set_size(titleBar, 480, 60);
  lv_obj_align(titleBar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_radius(titleBar, 0, 0);
  lv_obj_set_style_bg_color(titleBar, lv_color_hex(0x222222), 0);
  lv_obj_set_style_border_width(titleBar, 0, 0);
  lv_obj_set_style_pad_all(titleBar, 0, 0);

  lv_obj_t *title = lv_label_create(titleBar);
  lv_label_set_text(title, "Category");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_center(title);

  lv_obj_t *backBtn = CreateButton(categoryScreen, "Back", 80, 40);
  lv_obj_align(backBtn, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_event_cb(backBtn, BackBtnEvent, LV_EVENT_CLICKED, NULL);

  AddCategoryButton("All", AlbumCategoryAll, 100);
  AddCategoryButton("BLD", AlbumCategoryBuilding, 190);
  AddCategoryButton("SCN", AlbumCategoryScenery, 280);
  AddCategoryButton("GAM", AlbumCategoryGame, 370);
  AddCategoryButton("PER", AlbumCategoryPerson, 460);
  AddCategoryButton("ANI", AlbumCategoryAnimal, 550);
}

void ui_main_init(void) {
  CreateMainScreen();
  CreateCategoryScreen();
  PhotoView_BindImageObject(photoImg);
  lv_screen_load(mainScreen);
}
