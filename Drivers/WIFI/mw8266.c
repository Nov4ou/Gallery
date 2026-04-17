#include "mw8266.h"
#include <stdio.h>
#include <string.h>

Mw8266Status_t Mw8266_Init(Mw8266Handle_t *handle, UART_HandleTypeDef *uart,
                           uint8_t *rxDmaBuf, uint16_t rxDmaBufSize) {
  if (handle == NULL || uart == NULL || rxDmaBuf == NULL ||
      rxDmaBufSize == 0U) {
    return mw8266InvalidParam;
  }

  handle->uart = uart;
  handle->rxDmaBuf = rxDmaBuf;
  handle->rxDmaBufSize = rxDmaBufSize;
  handle->rxFrameLen = 0;
  handle->rxFrameReady = 0;

  memset(handle->rxDmaBuf, 0, handle->rxDmaBufSize);

  return mw8266Ok;
}

Mw8266Status_t Mw8266_StartReceive(Mw8266Handle_t *handle) {
  if (handle == NULL || handle->uart == NULL || handle->rxDmaBuf == NULL) {
    return mw8266InvalidParam;
  }

  handle->rxFrameLen = 0;
  handle->rxFrameReady = 0;
  memset(handle->rxDmaBuf, 0, handle->rxDmaBufSize);

  if (HAL_UART_Receive_DMA(handle->uart, handle->rxDmaBuf,
                           handle->rxDmaBufSize) != HAL_OK) {
    return mw8266Error;
  }

  __HAL_DMA_DISABLE_IT(handle->uart->hdmarx, DMA_IT_HT);
  __HAL_UART_ENABLE_IT(handle->uart, UART_IT_IDLE);

  return mw8266Ok;
}

void Mw8266_UartIdleHandler(Mw8266Handle_t *handle) {
  if (handle == NULL || handle->uart == NULL) {
    return;
  }

  if (__HAL_UART_GET_FLAG(handle->uart, UART_FLAG_IDLE) != RESET) {
    __HAL_UART_CLEAR_IDLEFLAG(handle->uart);

    HAL_UART_DMAStop(handle->uart);

    handle->rxFrameLen =
        handle->rxDmaBufSize - __HAL_DMA_GET_COUNTER(handle->uart->hdmarx);

    handle->rxFrameReady = 1;
  }
}

Mw8266Status_t Mw8266_ClearFrame(Mw8266Handle_t *handle) {
  if (handle == NULL || handle->rxDmaBuf == NULL) {
    return mw8266InvalidParam;
  }

  memset(handle->rxDmaBuf, 0, handle->rxDmaBufSize);
  handle->rxFrameLen = 0;
  handle->rxFrameReady = 0;

  return mw8266Ok;
}

Mw8266Status_t Mw8266_WaitFrame(Mw8266Handle_t *handle, uint32_t timeoutMs) {
  uint32_t startTick;

  if (handle == NULL) {
    return mw8266InvalidParam;
  }

  startTick = HAL_GetTick();

  while ((HAL_GetTick() - startTick) < timeoutMs) {
    if (handle->rxFrameReady) {
      handle->rxFrameReady = 0;

      if (handle->rxFrameLen < handle->rxDmaBufSize) {
        handle->rxDmaBuf[handle->rxFrameLen] = '\0';
      } else {
        handle->rxDmaBuf[handle->rxDmaBufSize - 1U] = '\0';
      }

      return mw8266Ok;
    }
  }

  return mw8266Timeout;
}

uint8_t Mw8266_FrameContains(Mw8266Handle_t *handle, const char *target) {
  if (handle == NULL || handle->rxDmaBuf == NULL || target == NULL) {
    return 0U;
  }

  if (strstr((char *)handle->rxDmaBuf, target) != NULL) {
    return 1U;
  }

  return 0U;
}

Mw8266Status_t Mw8266_SendRaw(Mw8266Handle_t *handle, const char *data) {
  if (handle == NULL || handle->uart == NULL || data == NULL) {
    return mw8266InvalidParam;
  }

  if (HAL_UART_Transmit(handle->uart, (uint8_t *)data, (uint16_t)strlen(data),
                        1000) != HAL_OK) {
    return mw8266Error;
  }

  return mw8266Ok;
}

Mw8266Status_t Mw8266_SendBytes(Mw8266Handle_t *handle, const uint8_t *data,
                                uint16_t len, uint32_t timeoutMs) {
  if (handle == NULL || handle->uart == NULL || data == NULL || len == 0U) {
    return mw8266InvalidParam;
  }

  if (HAL_UART_Transmit(handle->uart, (uint8_t *)data, len, timeoutMs) !=
      HAL_OK) {
    return mw8266Error;
  }

  return mw8266Ok;
}

Mw8266Status_t Mw8266_ExecCmd(Mw8266Handle_t *handle, const char *cmd,
                              const char *expect1, const char *expect2,
                              uint32_t timeoutMs) {
  Mw8266Status_t status;

  if (handle == NULL || cmd == NULL) {
    return mw8266InvalidParam;
  }

  status = Mw8266_ClearFrame(handle);
  if (status != mw8266Ok) {
    return status;
  }

  status = Mw8266_StartReceive(handle);
  if (status != mw8266Ok) {
    return status;
  }

  status = Mw8266_SendRaw(handle, cmd);
  if (status != mw8266Ok) {
    return status;
  }

  status = Mw8266_WaitFrame(handle, timeoutMs);
  if (status != mw8266Ok) {
    return status;
  }

  if (expect1 != NULL && strstr((char *)handle->rxDmaBuf, expect1) != NULL) {
    return mw8266Ok;
  }

  if (expect2 != NULL && strstr((char *)handle->rxDmaBuf, expect2) != NULL) {
    return mw8266Ok;
  }

  return mw8266Error;
}

Mw8266Status_t Mw8266_TestAT(Mw8266Handle_t *handle) {
  return Mw8266_ExecCmd(handle, "AT\r\n", "OK", NULL, 1000);
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

Mw8266Status_t Mw8266_SetMux(Mw8266Handle_t *handle, uint8_t enable) {
  char cmd[32];

  snprintf(cmd, sizeof(cmd), "AT+CIPMUX=%u\r\n", enable ? 1U : 0U);
  return Mw8266_ExecCmd(handle, cmd, "OK", NULL, 1000);
}

Mw8266Status_t Mw8266_StartServer(Mw8266Handle_t *handle, uint16_t port) {
  char cmd[32];

  snprintf(cmd, sizeof(cmd), "AT+CIPSERVER=1,%u\r\n", port);
  return Mw8266_ExecCmd(handle, cmd, "OK", NULL, 3000);
}

Mw8266Status_t Mw8266_StopServer(Mw8266Handle_t *handle) {
  return Mw8266_ExecCmd(handle, "AT+CIPSERVER=0\r\n", "OK", NULL, 3000);
}

Mw8266Status_t Mw8266_GetIP(Mw8266Handle_t *handle) {
  return Mw8266_ExecCmd(handle, "AT+CIFSR\r\n", "OK", NULL, 1000);
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

  status = Mw8266_SendBytes(handle, data, len, timeoutMs);
  if (status != mw8266Ok) {
    return status;
  }

  status = Mw8266_WaitFrame(handle, timeoutMs);
  if (status != mw8266Ok) {
    return status;
  }

  if (strstr((char *)handle->rxDmaBuf, "SEND OK") != NULL) {
    return mw8266Ok;
  }

  return mw8266Error;
}

int Mw8266_ParseIpd(uint8_t *buf, uint16_t bufLen, uint8_t *linkId,
                    uint16_t *dataLen, uint8_t **payload) {
  unsigned int id;
  unsigned int len;
  char *p;
  char *colon;

  (void)bufLen;

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

int Mw8266_IsIpdComplete(uint8_t *buf, uint16_t bufLen, uint8_t *payload,
                         uint16_t payloadLen, uint16_t *actualAvailableLen) {
  uint16_t offset;

  if (buf == NULL || payload == NULL || actualAvailableLen == NULL) {
    return -1;
  }

  if (payload < buf) {
    return -2;
  }

  offset = (uint16_t)(payload - buf);

  if (offset > bufLen) {
    return -3;
  }

  *actualAvailableLen = (uint16_t)(bufLen - offset);

  if (*actualAvailableLen >= payloadLen) {
    return 1;
  }

  return 0;
}