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
static lv_obj_t *searchScreen;
static lv_obj_t *photoImg;
static lv_obj_t *passwordLabel;
static lv_obj_t *passwordStatusLabel;
static lv_obj_t *searchInputLabel;
static lv_obj_t *searchStatusLabel;
static lv_obj_t *searchResultContainer;
static char passwordInput[PASSWORD_MAX_LEN + 1U];
static uint8_t passwordInputLen = 0;
static char searchInput = '\0';

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

static void SearchBtnEvent(lv_event_t *e) {
  (void)e;
  lv_screen_load(searchScreen);
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

static int CharEqualsIgnoreCase(char a, char b) {
  if (a >= 'A' && a <= 'Z') {
    a = (char)(a - 'A' + 'a');
  }
  if (b >= 'A' && b <= 'Z') {
    b = (char)(b - 'A' + 'a');
  }

  return a == b;
}

static int PathContainsChar(const char *path, char target) {
  if (path == NULL || target == '\0') {
    return 0;
  }

  while (*path != '\0') {
    if (CharEqualsIgnoreCase(*path, target)) {
      return 1;
    }
    path++;
  }

  return 0;
}

static const char *FileNameFromPath(const char *path) {
  const char *slash;

  if (path == NULL) {
    return "";
  }

  slash = strrchr(path, '/');
  if (slash == NULL) {
    return path;
  }

  return slash + 1;
}

static void SearchInputRefresh(void) {
  char text[2];

  if (searchInput == '\0') {
    lv_label_set_text(searchInputLabel, "");
    return;
  }

  text[0] = searchInput;
  text[1] = '\0';
  lv_label_set_text(searchInputLabel, text);
}

static void SearchResultEvent(lv_event_t *e) {
  uint32_t photoIndex;

  photoIndex = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
  Album_ShowByIndex(photoIndex);
  lv_screen_load(mainScreen);
}

static void SearchBuildResults(void) {
  uint32_t i;
  uint32_t resultCount = 0;
  char statusText[32];

  lv_obj_clean(searchResultContainer);

  if (searchInput == '\0') {
    lv_label_set_text(searchStatusLabel, "Input one character");
    return;
  }

  for (i = 0; i < gPhotoCount; i++) {
    if (PathContainsChar(gPhotoList[i], searchInput)) {
      lv_obj_t *btn;
      lv_obj_t *label;

      btn = lv_button_create(searchResultContainer);
      lv_obj_set_width(btn, 420);
      lv_obj_set_height(btn, 44);
      lv_obj_set_style_radius(btn, 8, 0);
      lv_obj_add_event_cb(btn, SearchResultEvent, LV_EVENT_CLICKED,
                          (void *)(uintptr_t)i);

      label = lv_label_create(btn);
      lv_label_set_text(label, FileNameFromPath(gPhotoList[i]));
      lv_obj_center(label);

      resultCount++;
    }
  }

  snprintf(statusText, sizeof(statusText), "Found %lu",
           (unsigned long)resultCount);
  lv_label_set_text(searchStatusLabel, statusText);
}

static void SearchKeyEvent(lv_event_t *e) {
  uintptr_t key;

  key = (uintptr_t)lv_event_get_user_data(e);

  if (key == 0U) {
    searchInput = '\0';
    lv_obj_clean(searchResultContainer);
    lv_label_set_text(searchStatusLabel, "Input one character");
  } else {
    searchInput = (char)key;
    SearchBuildResults();
  }

  SearchInputRefresh();
}

static void AddPasswordButton(const char *text, uintptr_t key, int16_t x,
                              int16_t y) {
  lv_obj_t *btn;

  btn = CreateButton(lockScreen, text, 90, 60);
  lv_obj_set_pos(btn, x, y);
  lv_obj_add_event_cb(btn, PasswordKeyEvent, LV_EVENT_CLICKED, (void *)key);
}

static void AddSearchKeyButton(const char *text, char key, int16_t x,
                               int16_t y) {
  lv_obj_t *btn;

  btn = CreateButton(searchScreen, text, 60, 38);
  lv_obj_set_pos(btn, x, y);
  lv_obj_add_event_cb(btn, SearchKeyEvent, LV_EVENT_CLICKED,
                      (void *)(uintptr_t)key);
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

  lv_obj_t *searchBtn = CreateButton(mainScreen, "Search", 90, 40);
  lv_obj_align(searchBtn, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_event_cb(searchBtn, SearchBtnEvent, LV_EVENT_CLICKED, NULL);

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

static void CreateSearchScreen(void) {
  static const char keys[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  uint32_t i;

  searchScreen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(searchScreen, lv_color_hex(0x111111), 0);
  lv_obj_set_style_bg_opa(searchScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(searchScreen, 0, 0);

  lv_obj_t *titleBar = lv_obj_create(searchScreen);
  lv_obj_set_size(titleBar, 480, 60);
  lv_obj_align(titleBar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_radius(titleBar, 0, 0);
  lv_obj_set_style_bg_color(titleBar, lv_color_hex(0x222222), 0);
  lv_obj_set_style_border_width(titleBar, 0, 0);
  lv_obj_set_style_pad_all(titleBar, 0, 0);

  lv_obj_t *title = lv_label_create(titleBar);
  lv_label_set_text(title, "Search");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_center(title);

  lv_obj_t *backBtn = CreateButton(searchScreen, "Back", 80, 40);
  lv_obj_align(backBtn, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_event_cb(backBtn, BackBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *inputBox = lv_obj_create(searchScreen);
  lv_obj_set_size(inputBox, 100, 48);
  lv_obj_align(inputBox, LV_ALIGN_TOP_MID, 0, 82);
  lv_obj_set_style_bg_color(inputBox, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_color(inputBox, lv_color_hex(0x666666), 0);
  lv_obj_set_style_border_width(inputBox, 2, 0);
  lv_obj_set_style_radius(inputBox, 8, 0);

  searchInputLabel = lv_label_create(inputBox);
  lv_label_set_text(searchInputLabel, "");
  lv_obj_set_style_text_color(searchInputLabel, lv_color_white(), 0);
  lv_obj_center(searchInputLabel);

  searchStatusLabel = lv_label_create(searchScreen);
  lv_label_set_text(searchStatusLabel, "Input one character");
  lv_obj_set_style_text_color(searchStatusLabel, lv_color_hex(0xDDDDDD), 0);
  lv_obj_align(searchStatusLabel, LV_ALIGN_TOP_MID, 0, 138);

  for (i = 0; i < sizeof(keys) - 1U; i++) {
    int16_t col = (int16_t)(i % 6U);
    int16_t row = (int16_t)(i / 6U);
    char text[2];

    text[0] = keys[i];
    text[1] = '\0';
    AddSearchKeyButton(text, keys[i], (int16_t)(25 + col * 72),
                       (int16_t)(170 + row * 48));
  }

  lv_obj_t *clearBtn = CreateButton(searchScreen, "CLR", 90, 38);
  lv_obj_set_pos(clearBtn, 195, 458);
  lv_obj_add_event_cb(clearBtn, SearchKeyEvent, LV_EVENT_CLICKED, NULL);

  searchResultContainer = lv_obj_create(searchScreen);
  lv_obj_set_size(searchResultContainer, 440, 270);
  lv_obj_align(searchResultContainer, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_bg_color(searchResultContainer, lv_color_hex(0x111111), 0);
  lv_obj_set_style_border_color(searchResultContainer, lv_color_hex(0x666666),
                                0);
  lv_obj_set_style_border_width(searchResultContainer, 2, 0);
  lv_obj_set_style_radius(searchResultContainer, 8, 0);
  lv_obj_set_scroll_dir(searchResultContainer, LV_DIR_VER);
  lv_obj_set_layout(searchResultContainer, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(searchResultContainer, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(searchResultContainer, 8, 0);
  lv_obj_set_style_pad_row(searchResultContainer, 8, 0);
}

void ui_main_init(void) {
  CreateLockScreen();
  CreateMainScreen();
  CreateCategoryScreen();
  CreateSearchScreen();
  PhotoView_BindImageObject(photoImg);
  LockGallery();
}
