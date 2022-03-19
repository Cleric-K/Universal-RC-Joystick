/*
 * protocol_common.c
 *
 *  Created on: 2.11.2018 �.
 *      Author: Cleric
 */
#include "stm32f1xx_hal.h"
#include "usbd_custom_hid_if.h"
#include "protocols.h"

uint16_t channels[MAX_CHANNELS];
uint32_t lastSentReportTime;
static JoyReport report;
int uartInvert;

/**
 * Waits for interframe interval of silence on the UART RX
 * 
 * iframe says how many ms must the line be silent before it is considered interframe
 * timeout is how many ms overall we are willing to wait for the silence, in case data is arriving
 * If 0, try only once
 * 
 * Returns 0 if no interframe interval was found (data was arriving more frequently than iframe)
 * Otherwise returns 1
 */
int ProtoWaitForInterframe(UART_HandleTypeDef* huart, uint32_t iframe, uint32_t timeout) {
  uint8_t buf[1];
  uint32_t time = HAL_GetTick() + timeout;
  HAL_StatusTypeDef ret;

  while((ret=HAL_UART_Receive(huart, buf, 1, iframe)) != HAL_TIMEOUT && timeout > 0) {
    if(HAL_GetTick() >= time)
      break;
  }
  return ret == HAL_TIMEOUT;
}


void BuildAndSendReport() {
  for(int i=0; i < NUM_AXES; i++) {
    uint16_t ch = channels[i];
    ch = MAX(1000, MIN(2000, ch)) - 1000;

    report.axes[i] = ch*0x7fff/1000;
  }

  // HID uses top left coordinates origin for X, Y
  // while radios have bottom left origin
  // so invert Y
  //report.axes[1] = 0x7fff - report.axes[1];

  uint8_t buttons = 0;
  for(int i=0; i < NUM_BUTTONS; i++) {
    if(channels[NUM_AXES + i] >= 1750)
      buttons |= 1<<i;
  }
  report.buttons = buttons;

  USB_SendReport(&report);
  lastSentReportTime = HAL_GetTick();
}

void ClearChannels() {
  memset(channels, 0, sizeof(channels));
}

void DebugLog(const char* buf) {
  int len = 0;
  const char* buf2 = buf;
  while(*buf2++) len++;
  while(HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, 1) == HAL_BUSY);
}

void ResetWatchdog() {
  HAL_IWDG_Refresh(&hiwdg);
}