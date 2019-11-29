/*
 * protocol_ppm.c
 *
 *  Created on: 5.11.2018 ã.
 *      Author: Cleric
 */


#include "stm32f1xx_hal.h"
#include "protocols.h"

extern UART_HandleTypeDef huart2;
void ProtoPpmReader(TIM_HandleTypeDef *htim) {
  ProtocolState state = INITIAL_INTERFRAME;
  int interFrame = 0;
  int32_t pulse;
  int chId = 0;
  uint32_t timeout = HAL_GetTick() + 100;

  HAL_TIM_Base_Start(htim);
  HAL_TIM_IC_Start(htim, TIM_CHANNEL_1);
  htim->Instance->CNT = 0;
  ClearChannels();

  while(1) {
    if(HAL_GetTick() > timeout)
      break;

    if(htim->Instance->CNT > 3000) {
      if(state == FRAME && chId > 0) {
        // got to the end of the frame
        BuildAndSendReport();
        timeout = HAL_GetTick() + 100;
        state = INITIAL_INTERFRAME;
      }
      interFrame = 1;
    }

    if(htim->Instance->SR & TIM_SR_CC1IF) {
      pulse = htim->Instance->CCR1;

      switch(state) {
      case INITIAL_INTERFRAME:
        if(interFrame) {
          interFrame = 0;
          chId = 0;
          state = FRAME;
        }
        //else
        //  HAL_UART_Transmit_DMA(&huart2, (uint8_t*)"pi\n", 3);
        break;

      case FRAME:
        if(pulse < 600 || pulse > 2400) {
          // invalid pulse width
          state = INITIAL_INTERFRAME;
          //HAL_UART_Transmit_DMA(&huart2, (uint8_t*)"pp\n", 3);
        }
        else {
          if(chId < MAX_CHANNELS)
            channels[chId++] = pulse;
        }
        break;

      }

    }
  }
}

