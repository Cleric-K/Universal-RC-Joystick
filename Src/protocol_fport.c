/*
 * protocol_fport.c
 *
 *  Created on: 30.11.2019
 *      Author: Cleric
 */

#include "stm32f1xx_hal.h"
#include "protocols.h"

#define FRAME_LEN 0xff
#define MAX_FAILS 4
#define MAGIC_BYTE 0x7e
#define ESCAPE_BYTE 0x7d
#define ESCAPE_MASK 0x20
#define CONTROL_FRAME 0x00
#define CRC_CORRECT 0xff


static int readEscapedStream(UART_HandleTypeDef* huart, uint8_t *buf, int num) {
  int ret = 0, i = -1;
  while(++i < num) {
    uint8_t byte;

    ret = HAL_UART_Receive(huart, &byte, 1, FRAME_TIMEOUT);
    if(ret != HAL_OK)
      break;
    if(byte == ESCAPE_BYTE) {
      // escape byte, we need one more
      ret = HAL_UART_Receive(huart, &byte, 1, FRAME_TIMEOUT);
      if(ret != HAL_OK)
        break;
      buf[i] = byte ^ ESCAPE_MASK;
    }
    else {
      buf[i] = byte;
    }
  }

  return ret == HAL_OK;
}




void ProtoFportReader(UART_HandleTypeDef* huart) {
  ProtocolState state = INITIAL_INTERFRAME;
  uint8_t buf[FRAME_LEN];
  int num_fails = 0;
  int failed;
  HAL_StatusTypeDef ret;
  int locked = 0;

  while(1) {
    if(num_fails >= MAX_FAILS)
      break;

    ResetWatchdog();

    // assume failed by default
    failed = 1;

    switch(state) {
    case INITIAL_INTERFRAME:
      if(ProtoWaitForInterframe(huart, INTERFRAME_MS, FRAME_TIMEOUT)) {
         // found interframe
         state = FRAME;
         failed = 0;
       }
       break;




    case FRAME:
      // get first byte
      ret = HAL_UART_Receive(huart, buf, 1, FRAME_TIMEOUT);
      if(ret != HAL_OK)
        break;

      // wrong first magic byte
      if(buf[0] != MAGIC_BYTE)
        break;

      // get the length byte
      if(!readEscapedStream(huart, &buf[1], 1))
        break;

      int data_len = buf[1];
      int i = 1;
      data_len += 4; // full frame length: add the first magic byte + len + crc + final magic byte

      // get the remaining frame
      if(!readEscapedStream(huart, &buf[2], data_len - 2 /* exclude first magic and length byte */))
        break;

      // check end magic byte
      if(buf[data_len - 1] != MAGIC_BYTE)
        break;

      if(buf[2] == CONTROL_FRAME) {
        // check crc
        uint16_t crc = 0;
				for(i=1; i < data_len - 1; i++) {
					crc += buf[i];
				}
        crc = (uint8_t)((crc & 0xff) + (crc >> 8));
        if(crc != CRC_CORRECT)
          break;

        DecodeSbusChannels(&buf[3]);
        BuildAndSendReport();

        if(!locked) {
          DebugLog(uartInvert ? "fpil\n" : "fpl\n");
          locked = 1;
        }

        num_fails = 0;
      }
      else {
        // if not control frame increment num_fails.
        // It is not really a failure, but this way we can detect if we are receving frames but never a control frame
        // Normally there should one control and one telemetry frame
        num_fails++;
      }

      failed = 0;

      break;
    }

    if(failed) {
      DebugLog(uartInvert ? "fpi\n" : "fp\n");
      num_fails++;
      state = INITIAL_INTERFRAME;
    }
  }
}
