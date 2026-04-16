#ifndef __MW8266_H
#define __MW8266_H

#include "main.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  mw8266Ok = 0,
  mw8266Error = 1,
  mw8266Timeout = 2,
  mw8266InvalidParam = 3
} Mw8266Status_t;

typedef struct {
  UART_HandleTypeDef *uart;
  uint8_t *rxBuf;
  uint16_t rxBufSize;
} Mw8266Handle_t;

/* Basic */
Mw8266Status_t Mw8266_Init(Mw8266Handle_t *handle, UART_HandleTypeDef *uart,
                           uint8_t *rxBuf, uint16_t rxBufSize);

Mw8266Status_t Mw8266_ClearBuffer(Mw8266Handle_t *handle);
Mw8266Status_t Mw8266_SendRaw(Mw8266Handle_t *handle, const char *data);
Mw8266Status_t Mw8266_ExecCmd(Mw8266Handle_t *handle, const char *cmd,
                              const char *expect, const char *expect2,
                              uint32_t timeoutMs);

/* Common AT commands */
Mw8266Status_t Mw8266_TestAT(Mw8266Handle_t *handle);
Mw8266Status_t Mw8266_Restart(Mw8266Handle_t *handle);
Mw8266Status_t Mw8266_SetEcho(Mw8266Handle_t *handle, uint8_t enable);
Mw8266Status_t Mw8266_SetWifiMode(Mw8266Handle_t *handle, uint8_t mode);
Mw8266Status_t Mw8266_JoinAP(Mw8266Handle_t *handle, const char *ssid,
                             const char *password, uint32_t timeoutMs);
Mw8266Status_t Mw8266_GetIP(Mw8266Handle_t *handle);

Mw8266Status_t Mw8266_ConfigAP(Mw8266Handle_t *handle, const char *ssid,
                               const char *password, uint8_t channel,
                               uint8_t ecn);
Mw8266Status_t Mw8266_StartServer(Mw8266Handle_t *handle, uint16_t port);
Mw8266Status_t Mw8266_StopServer(Mw8266Handle_t *handle);

Mw8266Status_t Mw8266_PollResponse(Mw8266Handle_t *handle, uint32_t timeoutMs);
Mw8266Status_t Mw8266_SendDataToClient(Mw8266Handle_t *handle, uint8_t linkId,
                                       const uint8_t *data, uint16_t len,
                                       uint32_t timeoutMs);

/* TCP / UDP */
Mw8266Status_t Mw8266_SetMux(Mw8266Handle_t *handle, uint8_t enable);
Mw8266Status_t Mw8266_StartTCP(Mw8266Handle_t *handle, const char *host,
                               uint16_t port);
Mw8266Status_t Mw8266_StartUDP(Mw8266Handle_t *handle, const char *host,
                               uint16_t port);
Mw8266Status_t Mw8266_Close(Mw8266Handle_t *handle);
Mw8266Status_t Mw8266_SendData(Mw8266Handle_t *handle, const uint8_t *data,
                               uint16_t len, uint32_t timeoutMs);

/* Utility */
uint8_t Mw8266_BufferContains(Mw8266Handle_t *handle, const char *target);

int Mw8266_ParseIpd(uint8_t *buf, uint16_t bufLen, uint8_t *linkId,
                    uint16_t *dataLen, uint8_t **payload);

#ifdef __cplusplus
}
#endif

#endif