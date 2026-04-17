#ifndef __MW8266_H
#define __MW8266_H

#include "main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    mw8266Ok = 0,
    mw8266Error,
    mw8266Timeout,
    mw8266InvalidParam
} Mw8266Status_t;

typedef struct
{
    UART_HandleTypeDef *uart;

    uint8_t *rxDmaBuf;
    uint16_t rxDmaBufSize;

    volatile uint16_t rxFrameLen;
    volatile uint8_t rxFrameReady;
} Mw8266Handle_t;

/* Init / RX */
Mw8266Status_t Mw8266_Init(Mw8266Handle_t *handle,
                           UART_HandleTypeDef *uart,
                           uint8_t *rxDmaBuf,
                           uint16_t rxDmaBufSize);

Mw8266Status_t Mw8266_StartReceive(Mw8266Handle_t *handle);
void Mw8266_UartIdleHandler(Mw8266Handle_t *handle);
Mw8266Status_t Mw8266_ClearFrame(Mw8266Handle_t *handle);
Mw8266Status_t Mw8266_WaitFrame(Mw8266Handle_t *handle, uint32_t timeoutMs);
uint8_t Mw8266_FrameContains(Mw8266Handle_t *handle, const char *target);

/* TX / command */
Mw8266Status_t Mw8266_SendRaw(Mw8266Handle_t *handle, const char *data);
Mw8266Status_t Mw8266_SendBytes(Mw8266Handle_t *handle,
                                const uint8_t *data,
                                uint16_t len,
                                uint32_t timeoutMs);

Mw8266Status_t Mw8266_ExecCmd(Mw8266Handle_t *handle,
                              const char *cmd,
                              const char *expect1,
                              const char *expect2,
                              uint32_t timeoutMs);

/* Basic AT */
Mw8266Status_t Mw8266_TestAT(Mw8266Handle_t *handle);
Mw8266Status_t Mw8266_SetEcho(Mw8266Handle_t *handle, uint8_t enable);
Mw8266Status_t Mw8266_SetWifiMode(Mw8266Handle_t *handle, uint8_t mode);
Mw8266Status_t Mw8266_ConfigAP(Mw8266Handle_t *handle,
                               const char *ssid,
                               const char *password,
                               uint8_t channel,
                               uint8_t ecn);
Mw8266Status_t Mw8266_SetMux(Mw8266Handle_t *handle, uint8_t enable);
Mw8266Status_t Mw8266_StartServer(Mw8266Handle_t *handle, uint16_t port);
Mw8266Status_t Mw8266_StopServer(Mw8266Handle_t *handle);
Mw8266Status_t Mw8266_GetIP(Mw8266Handle_t *handle);

/* Data send */
Mw8266Status_t Mw8266_SendDataToClient(Mw8266Handle_t *handle,
                                       uint8_t linkId,
                                       const uint8_t *data,
                                       uint16_t len,
                                       uint32_t timeoutMs);

/* +IPD parse */
int Mw8266_ParseIpd(uint8_t *buf,
                    uint16_t bufLen,
                    uint8_t *linkId,
                    uint16_t *dataLen,
                    uint8_t **payload);

int Mw8266_IsIpdComplete(uint8_t *buf,
                         uint16_t bufLen,
                         uint8_t *payload,
                         uint16_t payloadLen,
                         uint16_t *actualAvailableLen);

#ifdef __cplusplus
}
#endif

#endif