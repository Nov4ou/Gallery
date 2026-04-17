#include "ui_main.h"
#include "album.h"
#include "lvgl.h"
#include "photo_view.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define GALLERY_PASSWORD "1234"
#define PASSWORD_MAX_LEN 4U

static lv_obj_t *lockScreen;
static lv_obj_t *mainScreen;
static lv_obj_t *categoryScreen;
static lv_obj_t *photoImg;
static lv_obj_t *passwordLabel;
static lv_obj_t *passwordStatusLabel;
static char passwordInput[PASSWORD_MAX_LEN + 1U];
static uint8_t passwordInputLen = 0;

extern uint32_t gCurrentPhotoIndex;

typedef enum {
  PasswordKeyDel = 10,
  PasswordKeyOk,
  PasswordKeyClear,
} PasswordKey_t;

static void PasswordInputReset(void) {
  passwordInputLen = 0;
  memset(passwordInput, 0, sizeof(passwordInput));
}

static void PasswordInputRefresh(void) {
  char dots[PASSWORD_MAX_LEN + 1U];
  uint8_t i;

  for (i = 0; i < passwordInputLen; i++) {
    dots[i] = '*';
  }
  dots[passwordInputLen] = '\0';

  lv_label_set_text(passwordLabel, dots);
}

static void LockGallery(void) {
  PasswordInputReset();
  PasswordInputRefresh();
  lv_label_set_text(passwordStatusLabel, "");
  lv_screen_load(lockScreen);
}

static void PasswordKeyEvent(lv_event_t *e) {
  uintptr_t key;

  key = (uintptr_t)lv_event_get_user_data(e);

  if (key <= 9U) {
    if (passwordInputLen < PASSWORD_MAX_LEN) {
      passwordInput[passwordInputLen] = (char)('0' + key);
      passwordInputLen++;
      passwordInput[passwordInputLen] = '\0';
    }
    lv_label_set_text(passwordStatusLabel, "");
  } else if (key == PasswordKeyDel) {
    if (passwordInputLen > 0U) {
      passwordInputLen--;
      passwordInput[passwordInputLen] = '\0';
    }
    lv_label_set_text(passwordStatusLabel, "");
  } else if (key == PasswordKeyClear) {
    PasswordInputReset();
    lv_label_set_text(passwordStatusLabel, "");
  } else if (key == PasswordKeyOk) {
    if (strcmp(passwordInput, GALLERY_PASSWORD) == 0) {
      PasswordInputReset();
      PasswordInputRefresh();
      lv_screen_load(mainScreen);
      return;
    }

    PasswordInputReset();
    lv_label_set_text(passwordStatusLabel, "Wrong password");
  }

  PasswordInputRefresh();
}

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

static void LockBtnEvent(lv_event_t *e) {
  (void)e;
  LockGallery();
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

static void AddPasswordButton(const char *text, uintptr_t key, int16_t x,
                              int16_t y) {
  lv_obj_t *btn;

  btn = CreateButton(lockScreen, text, 90, 60);
  lv_obj_set_pos(btn, x, y);
  lv_obj_add_event_cb(btn, PasswordKeyEvent, LV_EVENT_CLICKED, (void *)key);
}

static void CreateLockScreen(void) {
  lockScreen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(lockScreen, lv_color_hex(0x111111), 0);
  lv_obj_set_style_bg_opa(lockScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(lockScreen, 0, 0);

  lv_obj_t *title = lv_label_create(lockScreen);
  lv_label_set_text(title, "Private Gallery");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 90);

  lv_obj_t *prompt = lv_label_create(lockScreen);
  lv_label_set_text(prompt, "Enter password");
  lv_obj_set_style_text_color(prompt, lv_color_hex(0xDDDDDD), 0);
  lv_obj_align(prompt, LV_ALIGN_TOP_MID, 0, 150);

  lv_obj_t *passwordBox = lv_obj_create(lockScreen);
  lv_obj_set_size(passwordBox, 260, 56);
  lv_obj_align(passwordBox, LV_ALIGN_TOP_MID, 0, 200);
  lv_obj_set_style_bg_color(passwordBox, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_color(passwordBox, lv_color_hex(0x666666), 0);
  lv_obj_set_style_border_width(passwordBox, 2, 0);
  lv_obj_set_style_radius(passwordBox, 8, 0);

  passwordLabel = lv_label_create(passwordBox);
  lv_label_set_text(passwordLabel, "");
  lv_obj_set_style_text_color(passwordLabel, lv_color_white(), 0);
  lv_obj_center(passwordLabel);

  passwordStatusLabel = lv_label_create(lockScreen);
  lv_label_set_text(passwordStatusLabel, "");
  lv_obj_set_style_text_color(passwordStatusLabel, lv_color_hex(0xFF6666), 0);
  lv_obj_align(passwordStatusLabel, LV_ALIGN_TOP_MID, 0, 270);

  AddPasswordButton("1", 1, 85, 330);
  AddPasswordButton("2", 2, 195, 330);
  AddPasswordButton("3", 3, 305, 330);
  AddPasswordButton("4", 4, 85, 410);
  AddPasswordButton("5", 5, 195, 410);
  AddPasswordButton("6", 6, 305, 410);
  AddPasswordButton("7", 7, 85, 490);
  AddPasswordButton("8", 8, 195, 490);
  AddPasswordButton("9", 9, 305, 490);
  AddPasswordButton("CLR", PasswordKeyClear, 85, 570);
  AddPasswordButton("0", 0, 195, 570);
  AddPasswordButton("Del", PasswordKeyDel, 305, 570);
  AddPasswordButton("OK", PasswordKeyOk, 195, 650);
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

  lv_obj_t *lockBtn = CreateButton(mainScreen, "Lock", 80, 40);
  lv_obj_align(lockBtn, LV_ALIGN_TOP_RIGHT, -10, 10);
  lv_obj_add_event_cb(lockBtn, LockBtnEvent, LV_EVENT_CLICKED, NULL);

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
  CreateLockScreen();
  CreateMainScreen();
  CreateCategoryScreen();
  PhotoView_BindImageObject(photoImg);
  LockGallery();
}
