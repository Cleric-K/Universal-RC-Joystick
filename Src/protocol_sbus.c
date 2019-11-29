/*
 * protocol_sbus.c
 *
 *  Created on: 2.11.2018 ã.
 *      Author: Cleric
 */

#include "stm32f1xx_hal.h"
#include "protocols.h"

#define FRAME_LEN 25
#define INTERFRAME_MS 1
#define MAX_FAILS 3
#define FRAME_FIRST_BYTE 0x0f

#define SBUS_CH_BITS 11
#define SBUS_CH_MASK  ((1<<SBUS_CH_BITS)-1);

extern UART_HandleTypeDef huart2;

static void DecodeChannels(uint8_t *buf) {
  int inputbits = 0;
  int inputbitsavailable = 0;

  for (int i=0; i<MAX_CHANNELS; i++) {
    while (inputbitsavailable < SBUS_CH_BITS) {
        inputbits |= *buf++ << inputbitsavailable;
        inputbitsavailable += 8;
    }

    int v = inputbits & SBUS_CH_MASK;


    // OpenTX sends channel data with in its own values. We can prescale them to the standard 1000 - 2000 range.
    // Thanks to @fape for providing the raw data:
    // min   mid   max
    // 172   992   1811

    // http://www.wolframalpha.com/input/?i=linear+fit+%7B172,+1000%7D,+%7B1811,+2000%7D,+%7B992,+1500%7D
    // slightly adjusted to give better results in integer math
    channels[i] = (610127*v + 895364000)/1000000;

    inputbitsavailable -= SBUS_CH_BITS;
    inputbits >>= SBUS_CH_BITS;
  }
}

void ProtoSbusReader(UART_HandleTypeDef* huart) {
  ProtocolState state = INITIAL_INTERFRAME;
  uint8_t buf[FRAME_LEN];
  int num_fails = 0;
  int failed;
  HAL_StatusTypeDef ret;

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
      ret = HAL_UART_Receive(huart, buf, FRAME_LEN, FRAME_TIMEOUT);
      if(ret != HAL_OK)
        break;

      // first and last byte have special values
      if(buf[0] != FRAME_FIRST_BYTE)
        // wrong values
        break;

      // frame looks ok. Decode channels
      DecodeChannels(&buf[1]);
      BuildAndSendReport();

      // make sure there are no additional bytes after frame
      if(ProtoWaitForInterframe(huart, INTERFRAME_MS, 0)) {
        // success
        failed = 0;
        num_fails = 0;
      }

      break;
    }

    if(failed) {
      //HAL_UART_Transmit_DMA(&huart2, (uint8_t*)"sb\n", 3);
      num_fails++;
      state = INITIAL_INTERFRAME;
    }
  }
}


