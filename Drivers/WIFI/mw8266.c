#include "mw8266.h"
#include <stdio.h>
#include <string.h>

static Mw8266Status_t Mw8266_ReadResponse(Mw8266Handle_t *handle,
                                          uint32_t timeoutMs);

Mw8266Status_t Mw8266_Init(Mw8266Handle_t *handle, UART_HandleTypeDef *uart,
                           uint8_t *rxBuf, uint16_t rxBufSize) {
  if (handle == NULL || uart == NULL || rxBuf == NULL || rxBufSize == 0U) {
    return mw8266InvalidParam;
  }

  handle->uart = uart;
  handle->rxBuf = rxBuf;
  handle->rxBufSize = rxBufSize;

  memset(handle->rxBuf, 0, handle->rxBufSize);
  return mw8266Ok;
}

Mw8266Status_t Mw8266_ClearBuffer(Mw8266Handle_t *handle) {
  if (handle == NULL || handle->rxBuf == NULL) {
    return mw8266InvalidParam;
  }

  memset(handle->rxBuf, 0, handle->rxBufSize);
  return mw8266Ok;
}

Mw8266Status_t Mw8266_SendRaw(Mw8266Handle_t *handle, const char *data) {
  if (handle == NULL || data == NULL) {
    return mw8266InvalidParam;
  }

  if (HAL_UART_Transmit(handle->uart, (uint8_t *)data, (uint16_t)strlen(data),
                        1000) != HAL_OK) {
    return mw8266Error;
  }

  return mw8266Ok;
}

uint8_t Mw8266_BufferContains(Mw8266Handle_t *handle, const char *target) {
  if (handle == NULL || handle->rxBuf == NULL || target == NULL) {
    return 0U;
  }

  return (strstr((char *)handle->rxBuf, target) != NULL) ? 1U : 0U;
}

static Mw8266Status_t Mw8266_ReadResponse(Mw8266Handle_t *handle,
                                          uint32_t timeoutMs) {
  uint16_t index = 0;
  uint8_t ch = 0;
  uint32_t startTick = HAL_GetTick();

  if (handle == NULL) {
    return mw8266InvalidParam;
  }

  memset(handle->rxBuf, 0, handle->rxBufSize);

  while ((HAL_GetTick() - startTick) < timeoutMs) {
    if (HAL_UART_Receive(handle->uart, &ch, 1, 10) == HAL_OK) {
      if (index < (handle->rxBufSize - 1U)) {
        handle->rxBuf[index++] = ch;
        handle->rxBuf[index] = '\0';
      }
    }
  }

  return mw8266Ok;
}

Mw8266Status_t Mw8266_ExecCmd(Mw8266Handle_t *handle, const char *cmd,
                              const char *expect, const char *expect2,
                              uint32_t timeoutMs) {
  Mw8266Status_t status;

  if (handle == NULL || cmd == NULL) {
    return mw8266InvalidParam;
  }

  Mw8266_ClearBuffer(handle);

  status = Mw8266_SendRaw(handle, cmd);
  if (status != mw8266Ok) {
    return status;
  }

  status = Mw8266_ReadResponse(handle, timeoutMs);
  if (status != mw8266Ok) {
    return status;
  }

  if (expect != NULL && Mw8266_BufferContains(handle, expect)) {
    return mw8266Ok;
  }

  if (expect2 != NULL && Mw8266_BufferContains(handle, expect2)) {
    return mw8266Ok;
  }

  return mw8266Timeout;
}

Mw8266Status_t Mw8266_TestAT(Mw8266Handle_t *handle) {
  return Mw8266_ExecCmd(handle, "AT\r\n", "OK", NULL, 1000);
}

Mw8266Status_t Mw8266_Restart(Mw8266Handle_t *handle) {
  return Mw8266_ExecCmd(handle, "AT+RST\r\n", "ready", "OK", 3000);
}

Mw8266Status_t Mw8266_SetEcho(Mw8266Handle_t *handle, uint8_t enable) {
  if (enable) {
    return Mw8266_ExecCmd(handle, "ATE1\r\n", "OK", NULL, 1000);
  } else {
    return Mw8266_ExecCmd(handle, "ATE0\r\n", "OK", NULL, 1000);
  }
}

Mw8266Status_t Mw8266_SetWifiMode(Mw8266Handle_t *handle, uint8_t mode) {
  char cmd[32];

  if (mode < 1U || mode > 3U) {
    return mw8266InvalidParam;
  }

  snprintf(cmd, sizeof(cmd), "AT+CWMODE=%u\r\n", mode);
  return Mw8266_ExecCmd(handle, cmd, "OK", NULL, 1000);
}

Mw8266Status_t Mw8266_JoinAP(Mw8266Handle_t *handle, const char *ssid,
                             const char *password, uint32_t timeoutMs) {
  char cmd[128];

  if (ssid == NULL || password == NULL) {
    return mw8266InvalidParam;
  }

  snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
  return Mw8266_ExecCmd(handle, cmd, "WIFI GOT IP", "OK", timeoutMs);
}

Mw8266Status_t Mw8266_GetIP(Mw8266Handle_t *handle) {
  return Mw8266_ExecCmd(handle, "AT+CIFSR\r\n", "STAIP", "192.168", 1000);
}

Mw8266Status_t Mw8266_ConfigAP(Mw8266Handle_t *handle, const char *ssid,
                               const char *password, uint8_t channel,
                               uint8_t ecn) {
  char cmd[128];

  if (handle == NULL || ssid == NULL || password == NULL) {
    return mw8266InvalidParam;
  }

  snprintf(cmd, sizeof(cmd), "AT+CWSAP=\"%s\",\"%s\",%u,%u\r\n", ssid, password,
           channel, ecn);

  return Mw8266_ExecCmd(handle, cmd, "OK", NULL, 3000);
}

Mw8266Status_t Mw8266_StopServer(Mw8266Handle_t *handle) {
  return Mw8266_ExecCmd(handle, "AT+CIPSERVER=0\r\n", "OK", NULL, 3000);
}

Mw8266Status_t Mw8266_PollResponse(Mw8266Handle_t *handle, uint32_t timeoutMs) {
  return Mw8266_ReadResponse(handle, timeoutMs);
}

Mw8266Status_t Mw8266_SendDataToClient(Mw8266Handle_t *handle, uint8_t linkId,
                                       const uint8_t *data, uint16_t len,
                                       uint32_t timeoutMs) {
  char cmd[32];
  Mw8266Status_t status;

  if (handle == NULL || data == NULL || len == 0U) {
    return mw8266InvalidParam;
  }

  snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u,%u\r\n", linkId, len);

  status = Mw8266_ExecCmd(handle, cmd, ">", NULL, 2000);
  if (status != mw8266Ok) {
    return status;
  }

  if (HAL_UART_Transmit(handle->uart, (uint8_t *)data, len, timeoutMs) !=
      HAL_OK) {
    return mw8266Error;
  }

  Mw8266_ClearBuffer(handle);
  status = Mw8266_ReadResponse(handle, timeoutMs);

  if (status != mw8266Ok) {
    return status;
  }

  if (Mw8266_BufferContains(handle, "SEND OK")) {
    return mw8266Ok;
  }

  return mw8266Timeout;
}

Mw8266Status_t Mw8266_StartServer(Mw8266Handle_t *handle, uint16_t port) {
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "AT+CIPSERVER=1,%u\r\n", port);
  return Mw8266_ExecCmd(handle, cmd, "OK", NULL, 3000);
}

Mw8266Status_t Mw8266_SetMux(Mw8266Handle_t *handle, uint8_t enable) {
  char cmd[32];

  snprintf(cmd, sizeof(cmd), "AT+CIPMUX=%u\r\n", enable ? 1U : 0U);
  return Mw8266_ExecCmd(handle, cmd, "OK", NULL, 1000);
}

Mw8266Status_t Mw8266_StartTCP(Mw8266Handle_t *handle, const char *host,
                               uint16_t port) {
  char cmd[128];

  if (host == NULL) {
    return mw8266InvalidParam;
  }

  snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u\r\n", host, port);
  return Mw8266_ExecCmd(handle, cmd, "CONNECT", "OK", 5000);
}

Mw8266Status_t Mw8266_StartUDP(Mw8266Handle_t *handle, const char *host,
                               uint16_t port) {
  char cmd[128];

  if (host == NULL) {
    return mw8266InvalidParam;
  }

  snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"UDP\",\"%s\",%u\r\n", host, port);
  return Mw8266_ExecCmd(handle, cmd, "CONNECT", "OK", 5000);
}

Mw8266Status_t Mw8266_Close(Mw8266Handle_t *handle) {
  return Mw8266_ExecCmd(handle, "AT+CIPCLOSE\r\n", "OK", "CLOSED", 2000);
}

Mw8266Status_t Mw8266_SendData(Mw8266Handle_t *handle, const uint8_t *data,
                               uint16_t len, uint32_t timeoutMs) {
  char cmd[32];
  Mw8266Status_t status;

  if (handle == NULL || data == NULL || len == 0U) {
    return mw8266InvalidParam;
  }

  snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u\r\n", len);

  status = Mw8266_ExecCmd(handle, cmd, ">", NULL, 2000);
  if (status != mw8266Ok) {
    return status;
  }

  if (HAL_UART_Transmit(handle->uart, (uint8_t *)data, len, timeoutMs) !=
      HAL_OK) {
    return mw8266Error;
  }

  Mw8266_ClearBuffer(handle);
  status = Mw8266_ReadResponse(handle, timeoutMs);

  if (status != mw8266Ok) {
    return status;
  }

  if (Mw8266_BufferContains(handle, "SEND OK")) {
    return mw8266Ok;
  }

  return mw8266Timeout;
}

int Mw8266_ParseIpd(uint8_t *buf, uint16_t bufLen, uint8_t *linkId,
                    uint16_t *dataLen, uint8_t **payload) {
  unsigned int id, len;
  char *p;
  char *colon;

  if (buf == NULL || linkId == NULL || dataLen == NULL || payload == NULL) {
    return -1;
  }

  p = strstr((char *)buf, "+IPD,");
  if (p == NULL) {
    return -2;
  }

  if (sscanf(p, "+IPD,%u,%u", &id, &len) != 2) {
    return -3;
  }

  colon = strchr(p, ':');
  if (colon == NULL) {
    return -4;
  }

  *linkId = (uint8_t)id;
  *dataLen = (uint16_t)len;
  *payload = (uint8_t *)(colon + 1);

  return 0;
}