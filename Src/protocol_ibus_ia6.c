/*
 * protocol_ibus_ia6.c
 *
 *  Created on: 4.11.2018 ã.
 *      Author: Cleric
 */


#include "stm32f1xx_hal.h"
#include "protocols.h"

#define FRAME_LEN 31
#define INTERFRAME_MS 1
#define MAX_FAILS 3

#define IBUS_MAGIC 0x55

#define READ_UINT16(buf, i) (buf[i] | (buf[i+1]<<8))



void ProtoIbusIa6Reader(UART_HandleTypeDef* huart) {
  ProtocolState state = INITIAL_INTERFRAME;
  uint8_t buf[FRAME_LEN];
  int num_fails = 0;
  int len = 0;
  uint16_t checksum = 0;
  int idx = 0;
  int i;
  int failed;

  while(1) {
    if(num_fails >= MAX_FAILS)
      break;

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
      if(HAL_UART_Receive(huart, buf, FRAME_LEN, FRAME_TIMEOUT) != HAL_OK)
          break;

      if(buf[0] != IBUS_MAGIC)
        break;

      // init checksum
      checksum = 0;

      // read channels
      idx=1;
      i=0;
      while(idx < len-3 /*exclude checksum and command byte*/) {
        if(i < MAX_CHANNELS) {
          channels[i] = READ_UINT16(buf, idx) & 0xfff;
        }
        checksum += channels[i];

        idx += 2;
        i++;
      }

      // validate checksum
      if(checksum == READ_UINT16(buf, idx)) {
        // checksum valid
        BuildAndSendReport();
      }

      // make sure there are no additional bytes after frame
      if(ProtoWaitForInterframe(huart, INTERFRAME_MS, 0)) {
        // success
        failed = 0;
        num_fails = 0;
      }

      break;
    }

    if(failed) {
      num_fails++;
      state = INITIAL_INTERFRAME;
    }
  }
}


