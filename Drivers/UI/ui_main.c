#include "audio_player.h"
#include "ui_main.h"
#include "album.h"
#include "lvgl.h"
#include "photo_view.h"
#include "ui_font_zh.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define GALLERY_PASSWORD "1234"
#define PASSWORD_MAX_LEN 4U
#define AUTOPLAY_INTERVAL_MS 3000U

#define TXT_STOP "\xE5\x81\x9C\xE6\xAD\xA2"
#define TXT_AUTO "\xE8\x87\xAA\xE5\x8A\xA8"
#define TXT_WRONG_PASSWORD "\xE5\xAF\x86\xE7\xA0\x81\xE9\x94\x99\xE8\xAF\xAF"
#define TXT_INPUT_CHAR "\xE8\xAF\xB7\xE8\xBE\x93\xE5\x85\xA5\xE5\xAD\x97\xE7\xAC\xA6"
#define TXT_FOUND "\xE6\x89\xBE\xE5\x88\xB0"
#define TXT_PRIVATE_GALLERY "\xE7\xA7\x81\xE5\xAF\x86\xE7\x9B\xB8\xE5\x86\x8C"
#define TXT_ENTER_PASSWORD "\xE8\xBE\x93\xE5\x85\xA5\xE5\xAF\x86\xE7\xA0\x81"
#define TXT_CLEAR "\xE6\xB8\x85\xE7\xA9\xBA"
#define TXT_DELETE "\xE5\x88\xA0\xE9\x99\xA4"
#define TXT_OK "\xE7\xA1\xAE\xE5\xAE\x9A"
#define TXT_GALLERY "\xE7\x9B\xB8\xE5\x86\x8C"
#define TXT_LOCK "\xE9\x94\x81\xE5\xAE\x9A"
#define TXT_SEARCH "\xE6\x90\x9C\xE7\xB4\xA2"
#define TXT_PREV "\xE4\xB8\x8A\xE4\xB8\x80\xE5\xBC\xA0"
#define TXT_NEXT "\xE4\xB8\x8B\xE4\xB8\x80\xE5\xBC\xA0"
#define TXT_CATEGORY "\xE5\x88\x86\xE7\xB1\xBB"
#define TXT_BACK "\xE8\xBF\x94\xE5\x9B\x9E"
#define TXT_ALL "\xE5\x85\xA8\xE9\x83\xA8"
#define TXT_BUILDING "\xE5\xBB\xBA\xE7\xAD\x91"
#define TXT_SCENERY "\xE6\x99\xAF\xE8\x89\xB2"
#define TXT_PLANT "\xE6\xA4\x8D\xE7\x89\xA9"
#define TXT_PERSON "\xE4\xBA\xBA\xE7\x89\xA9"
#define TXT_ANIMAL "\xE5\x8A\xA8\xE7\x89\xA9"
#define TXT_MUSIC "\xE9\x9F\xB3\xE4\xB9\x90"
#define TXT_PLAY "\xE6\x92\xAD\xE6\x94\xBE"
#define TXT_NO_WAV "\xE6\x9C\xAA\xE6\x89\xBE\xE5\x88\xB0WAV"
#define TXT_PREV_TRACK "\xE4\xB8\x8A\xE4\xB8\x80\xE9\xA6\x96"
#define TXT_NEXT_TRACK "\xE4\xB8\x8B\xE4\xB8\x80\xE9\xA6\x96"

static lv_obj_t *lockScreen;
static lv_obj_t *mainScreen;
static lv_obj_t *categoryScreen;
static lv_obj_t *searchScreen;
static lv_obj_t *musicScreen;
static lv_obj_t *photoImg;
static lv_obj_t *photoInfoLabel;
static lv_obj_t *autoplayLabel;
static lv_obj_t *passwordLabel;
static lv_obj_t *passwordStatusLabel;
static lv_obj_t *searchInputLabel;
static lv_obj_t *searchStatusLabel;
static lv_obj_t *searchResultContainer;
static lv_obj_t *musicNowPlayingLabel;
static lv_obj_t *musicPlayLabel;
static lv_obj_t *musicListContainer;
static lv_timer_t *autoplayTimer;
static char passwordInput[PASSWORD_MAX_LEN + 1U];
static uint8_t passwordInputLen = 0;
static char searchInput = '\0';
static uint8_t autoplayEnabled = 0;

extern uint32_t gCurrentPhotoIndex;

typedef enum {
  PasswordKeyDel = 10,
  PasswordKeyOk,
  PasswordKeyClear,
} PasswordKey_t;

static void AutoplayStop(void);
static void MusicRefreshUi(void);

static void ApplyUIFont(lv_obj_t *obj) {
  const lv_font_t *font = UI_FontZhGet();

  if (obj != NULL && font != NULL) {
    lv_obj_set_style_text_font(obj, font, 0);
  }
}

static void ApplyUIFontTree(lv_obj_t *obj) {
  uint32_t i;
  uint32_t childCount;

  if (obj == NULL) {
    return;
  }

  ApplyUIFont(obj);
  childCount = lv_obj_get_child_count(obj);

  for (i = 0; i < childCount; i++) {
    ApplyUIFontTree(lv_obj_get_child(obj, (int32_t)i));
  }
}

static void UpdatePhotoInfoLabel(uint32_t index) {
  char label[ALBUM_LABEL_MAX];

  if (photoInfoLabel == NULL) {
    return;
  }

  if (Album_FormatPhotoLabel(index, label, sizeof(label)) == 0) {
    lv_label_set_text(photoInfoLabel, label);
  } else {
    lv_label_set_text(photoInfoLabel, "");
  }
}

static void PhotoChangedCallback(uint32_t index) { UpdatePhotoInfoLabel(index); }

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
  AutoplayStop();
  PasswordInputReset();
  PasswordInputRefresh();
  lv_label_set_text(passwordStatusLabel, "");
  lv_screen_load(lockScreen);
}

static void AutoplayRefreshLabel(void) {
  if (autoplayLabel == NULL) {
    return;
  }

  if (autoplayEnabled) {
    lv_label_set_text(autoplayLabel, TXT_STOP);
  } else {
    lv_label_set_text(autoplayLabel, TXT_AUTO);
  }
}

static void AutoplayTimerCb(lv_timer_t *timer) {
  (void)timer;

  if (!autoplayEnabled) {
    return;
  }

  if (lv_screen_active() != mainScreen) {
    return;
  }

  Album_ShowNext();
}

static void AutoplayStart(void) {
  if (autoplayTimer == NULL) {
    autoplayTimer = lv_timer_create(AutoplayTimerCb, AUTOPLAY_INTERVAL_MS, NULL);
  }

  autoplayEnabled = 1U;
  AutoplayRefreshLabel();
}

static void AutoplayStop(void) {
  if (autoplayTimer != NULL) {
    lv_timer_delete(autoplayTimer);
    autoplayTimer = NULL;
  }

  autoplayEnabled = 0U;
  AutoplayRefreshLabel();
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
    lv_label_set_text(passwordStatusLabel, TXT_WRONG_PASSWORD);
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

static void AutoplayBtnEvent(lv_event_t *e) {
  (void)e;

  if (autoplayEnabled) {
    AutoplayStop();
  } else {
    AutoplayStart();
  }
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

static void MusicStateChangedCallback(void) { MusicRefreshUi(); }

static void MusicBackBtnEvent(lv_event_t *e) {
  (void)e;
  lv_screen_load(categoryScreen);
}

static void MusicTrackEvent(lv_event_t *e) {
  uint32_t trackIndex = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
  (void)AudioPlayer_PlayTrack(trackIndex);
}

static void MusicPrevBtnEvent(lv_event_t *e) {
  (void)e;
  (void)AudioPlayer_PlayPrevTrack();
}

static void MusicPlayBtnEvent(lv_event_t *e) {
  (void)e;

  if (AudioPlayer_IsPlaying() != 0U) {
    AudioPlayer_StopPlayback();
  } else {
    (void)AudioPlayer_PlayCurrent();
  }
}

static void MusicNextBtnEvent(lv_event_t *e) {
  (void)e;
  (void)AudioPlayer_PlayNextTrack();
}

static void MusicBtnEvent(lv_event_t *e) {
  (void)e;
  AudioPlayer_ScanTracks();
  MusicRefreshUi();
  lv_screen_load(musicScreen);
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
  ApplyUIFont(label);
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
    lv_label_set_text(searchStatusLabel, TXT_INPUT_CHAR);
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
      ApplyUIFont(label);
      lv_obj_center(label);

      resultCount++;
    }
  }

  snprintf(statusText, sizeof(statusText), TXT_FOUND " %lu",
           (unsigned long)resultCount);
  lv_label_set_text(searchStatusLabel, statusText);
}

static void SearchKeyEvent(lv_event_t *e) {
  uintptr_t key;

  key = (uintptr_t)lv_event_get_user_data(e);

  if (key == 0U) {
    searchInput = '\0';
    lv_obj_clean(searchResultContainer);
    lv_label_set_text(searchStatusLabel, TXT_INPUT_CHAR);
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

static void MusicRefreshUi(void) {
  uint32_t count;
  int32_t currentIndex;
  uint8_t isPlaying;
  char currentName[64];
  if (musicNowPlayingLabel == NULL || musicListContainer == NULL ||
      musicPlayLabel == NULL) {
    return;
  }

  count = AudioPlayer_GetTrackCount();
  currentIndex = AudioPlayer_GetCurrentTrackIndex();
  isPlaying = AudioPlayer_IsPlaying();

  if (isPlaying != 0U) {
    lv_label_set_text(musicPlayLabel, TXT_STOP);
  } else {
    lv_label_set_text(musicPlayLabel, TXT_PLAY);
  }

  if (count == 0U) {
    lv_label_set_text(musicNowPlayingLabel, TXT_NO_WAV);
    lv_obj_clean(musicListContainer);
    return;
  }

  if (currentIndex >= 0 &&
      AudioPlayer_GetTrackName((uint32_t)currentIndex, currentName,
                               sizeof(currentName)) == 0) {
    lv_label_set_text(musicNowPlayingLabel, currentName);
  } else {
    lv_label_set_text(musicNowPlayingLabel, "");
  }

  lv_obj_clean(musicListContainer);

  for (uint32_t i = 0; i < count; i++) {
    lv_obj_t *btn;
    lv_obj_t *label;
    char name[64];

    if (AudioPlayer_GetTrackName(i, name, sizeof(name)) != 0) {
      snprintf(name, sizeof(name), "Track %lu", (unsigned long)(i + 1U));
    }

    btn = lv_button_create(musicListContainer);
    lv_obj_set_width(btn, 420);
    lv_obj_set_height(btn, 50);
    lv_obj_set_style_radius(btn, 8, 0);

    if ((int32_t)i == currentIndex) {
      lv_obj_set_style_bg_color(btn,
                                isPlaying ? lv_color_hex(0x2E7D32)
                                          : lv_color_hex(0x444444),
                                0);
    }

    lv_obj_add_event_cb(btn, MusicTrackEvent, LV_EVENT_CLICKED,
                        (void *)(uintptr_t)i);

    label = lv_label_create(btn);
    lv_label_set_text(label, name);
    ApplyUIFont(label);
    lv_obj_center(label);
  }
}

static void CreateLockScreen(void) {
  lockScreen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(lockScreen, lv_color_hex(0x111111), 0);
  lv_obj_set_style_bg_opa(lockScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(lockScreen, 0, 0);

  lv_obj_t *title = lv_label_create(lockScreen);
  lv_label_set_text(title, TXT_PRIVATE_GALLERY);
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 90);

  lv_obj_t *prompt = lv_label_create(lockScreen);
  lv_label_set_text(prompt, TXT_ENTER_PASSWORD);
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
  AddPasswordButton(TXT_CLEAR, PasswordKeyClear, 85, 570);
  AddPasswordButton("0", 0, 195, 570);
  AddPasswordButton(TXT_DELETE, PasswordKeyDel, 305, 570);
  AddPasswordButton(TXT_OK, PasswordKeyOk, 195, 650);
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
  lv_label_set_text(title, TXT_GALLERY);
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_center(title);

  lv_obj_t *lockBtn = CreateButton(mainScreen, TXT_LOCK, 80, 40);
  lv_obj_align(lockBtn, LV_ALIGN_TOP_RIGHT, -10, 10);
  lv_obj_add_event_cb(lockBtn, LockBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *searchBtn = CreateButton(mainScreen, TXT_SEARCH, 90, 40);
  lv_obj_align(searchBtn, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_event_cb(searchBtn, SearchBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *autoplayBtn = lv_button_create(mainScreen);
  lv_obj_set_size(autoplayBtn, 80, 40);
  lv_obj_align(autoplayBtn, LV_ALIGN_TOP_LEFT, 110, 10);
  lv_obj_set_style_radius(autoplayBtn, 8, 0);
  lv_obj_add_event_cb(autoplayBtn, AutoplayBtnEvent, LV_EVENT_CLICKED, NULL);

  autoplayLabel = lv_label_create(autoplayBtn);
  lv_label_set_text(autoplayLabel, TXT_AUTO);
  ApplyUIFont(autoplayLabel);
  lv_obj_center(autoplayLabel);

  lv_obj_t *photoPanel = lv_obj_create(mainScreen);
  lv_obj_set_size(photoPanel, 440, 620);
  lv_obj_align(photoPanel, LV_ALIGN_TOP_MID, 0, 80);
  lv_obj_set_style_bg_color(photoPanel, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_color(photoPanel, lv_color_hex(0x666666), 0);
  lv_obj_set_style_border_width(photoPanel, 2, 0);
  lv_obj_set_style_radius(photoPanel, 8, 0);

  photoImg = lv_image_create(photoPanel);
  lv_obj_center(photoImg);

  lv_obj_t *infoBar = lv_obj_create(photoPanel);
  lv_obj_set_size(infoBar, 420, 54);
  lv_obj_align(infoBar, LV_ALIGN_BOTTOM_MID, 0, -8);
  lv_obj_set_style_bg_color(infoBar, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(infoBar, LV_OPA_70, 0);
  lv_obj_set_style_border_width(infoBar, 0, 0);
  lv_obj_set_style_radius(infoBar, 8, 0);
  lv_obj_set_style_pad_all(infoBar, 8, 0);

  photoInfoLabel = lv_label_create(infoBar);
  lv_label_set_text(photoInfoLabel, "");
  lv_label_set_long_mode(photoInfoLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(photoInfoLabel, 400);
  lv_obj_set_style_text_color(photoInfoLabel, lv_color_white(), 0);
  lv_obj_center(photoInfoLabel);

  lv_obj_t *prevBtn = CreateButton(mainScreen, TXT_PREV, 100, 50);
  lv_obj_align(prevBtn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
  lv_obj_add_event_cb(prevBtn, PrevBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *nextBtn = CreateButton(mainScreen, TXT_NEXT, 100, 50);
  lv_obj_align(nextBtn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
  lv_obj_add_event_cb(nextBtn, NextBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *categoryBtn = CreateButton(mainScreen, TXT_CATEGORY, 140, 50);
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
  lv_label_set_text(title, TXT_CATEGORY);
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_center(title);

  lv_obj_t *backBtn = CreateButton(categoryScreen, TXT_BACK, 80, 40);
  lv_obj_align(backBtn, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_event_cb(backBtn, BackBtnEvent, LV_EVENT_CLICKED, NULL);

  AddCategoryButton(TXT_ALL, AlbumCategoryAll, 100);
  AddCategoryButton(TXT_BUILDING, AlbumCategoryBuilding, 190);
  AddCategoryButton(TXT_SCENERY, AlbumCategoryScenery, 280);
  AddCategoryButton(TXT_PLANT, AlbumCategoryPlant, 370);
  AddCategoryButton(TXT_PERSON, AlbumCategoryPerson, 460);
  AddCategoryButton(TXT_ANIMAL, AlbumCategoryAnimal, 550);

  lv_obj_t *musicBtn = CreateButton(categoryScreen, TXT_MUSIC, 260, 70);
  lv_obj_align(musicBtn, LV_ALIGN_TOP_MID, 0, 640);
  lv_obj_add_event_cb(musicBtn, MusicBtnEvent, LV_EVENT_CLICKED, NULL);
}

static void CreateMusicScreen(void) {
  musicScreen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(musicScreen, lv_color_hex(0x111111), 0);
  lv_obj_set_style_bg_opa(musicScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(musicScreen, 0, 0);

  lv_obj_t *titleBar = lv_obj_create(musicScreen);
  lv_obj_set_size(titleBar, 480, 60);
  lv_obj_align(titleBar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_radius(titleBar, 0, 0);
  lv_obj_set_style_bg_color(titleBar, lv_color_hex(0x222222), 0);
  lv_obj_set_style_border_width(titleBar, 0, 0);
  lv_obj_set_style_pad_all(titleBar, 0, 0);

  lv_obj_t *title = lv_label_create(titleBar);
  lv_label_set_text(title, TXT_MUSIC);
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_center(title);

  lv_obj_t *backBtn = CreateButton(musicScreen, TXT_BACK, 80, 40);
  lv_obj_align(backBtn, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_event_cb(backBtn, MusicBackBtnEvent, LV_EVENT_CLICKED, NULL);

  musicNowPlayingLabel = lv_label_create(musicScreen);
  lv_label_set_text(musicNowPlayingLabel, TXT_NO_WAV);
  lv_label_set_long_mode(musicNowPlayingLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(musicNowPlayingLabel, 420);
  lv_obj_align(musicNowPlayingLabel, LV_ALIGN_TOP_MID, 0, 84);
  lv_obj_set_style_text_color(musicNowPlayingLabel, lv_color_white(), 0);

  lv_obj_t *prevBtn = CreateButton(musicScreen, TXT_PREV_TRACK, 100, 46);
  lv_obj_align(prevBtn, LV_ALIGN_TOP_LEFT, 20, 128);
  lv_obj_add_event_cb(prevBtn, MusicPrevBtnEvent, LV_EVENT_CLICKED, NULL);

  lv_obj_t *playBtn = CreateButton(musicScreen, TXT_PLAY, 100, 46);
  lv_obj_align(playBtn, LV_ALIGN_TOP_MID, 0, 128);
  lv_obj_add_event_cb(playBtn, MusicPlayBtnEvent, LV_EVENT_CLICKED, NULL);
  musicPlayLabel = lv_obj_get_child(playBtn, 0);

  lv_obj_t *nextBtn = CreateButton(musicScreen, TXT_NEXT_TRACK, 100, 46);
  lv_obj_align(nextBtn, LV_ALIGN_TOP_RIGHT, -20, 128);
  lv_obj_add_event_cb(nextBtn, MusicNextBtnEvent, LV_EVENT_CLICKED, NULL);

  musicListContainer = lv_obj_create(musicScreen);
  lv_obj_set_size(musicListContainer, 440, 560);
  lv_obj_align(musicListContainer, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_bg_color(musicListContainer, lv_color_hex(0x111111), 0);
  lv_obj_set_style_border_color(musicListContainer, lv_color_hex(0x666666), 0);
  lv_obj_set_style_border_width(musicListContainer, 2, 0);
  lv_obj_set_style_radius(musicListContainer, 8, 0);
  lv_obj_set_scroll_dir(musicListContainer, LV_DIR_VER);
  lv_obj_set_layout(musicListContainer, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(musicListContainer, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(musicListContainer, 8, 0);
  lv_obj_set_style_pad_row(musicListContainer, 8, 0);
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
  lv_label_set_text(title, TXT_SEARCH);
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_center(title);

  lv_obj_t *backBtn = CreateButton(searchScreen, TXT_BACK, 80, 40);
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
  lv_label_set_text(searchStatusLabel, TXT_INPUT_CHAR);
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

  lv_obj_t *clearBtn = CreateButton(searchScreen, TXT_CLEAR, 90, 38);
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
  UI_FontZhInit();
  CreateLockScreen();
  CreateMainScreen();
  CreateCategoryScreen();
  CreateMusicScreen();
  CreateSearchScreen();
  ApplyUIFontTree(lockScreen);
  ApplyUIFontTree(mainScreen);
  ApplyUIFontTree(categoryScreen);
  ApplyUIFontTree(musicScreen);
  ApplyUIFontTree(searchScreen);
  PhotoView_BindImageObject(photoImg);
  Album_SetPhotoChangedCallback(PhotoChangedCallback);
  AudioPlayer_SetStateChangedCallback(MusicStateChangedCallback);
  MusicRefreshUi();
  UpdatePhotoInfoLabel(gCurrentPhotoIndex);
  LockGallery();
}
