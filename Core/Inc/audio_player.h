#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef void (*AudioPlayerStateChangedCallback_t)(void);

void AudioPlayer_ScanTracks(void);
uint32_t AudioPlayer_GetTrackCount(void);
int32_t AudioPlayer_GetCurrentTrackIndex(void);
uint8_t AudioPlayer_IsPlaying(void);
int AudioPlayer_GetTrackName(uint32_t index, char *buffer, uint32_t bufferSize);
HAL_StatusTypeDef AudioPlayer_PlayTrack(uint32_t index);
HAL_StatusTypeDef AudioPlayer_PlayCurrent(void);
HAL_StatusTypeDef AudioPlayer_PlayNextTrack(void);
HAL_StatusTypeDef AudioPlayer_PlayPrevTrack(void);
void AudioPlayer_StopPlayback(void);
void AudioPlayer_SetStateChangedCallback(AudioPlayerStateChangedCallback_t callback);

#endif
