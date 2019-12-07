/*
 * protocols.h
 *
 *  Created on: 2.11.2018 �.
 *      Author: Cleric
 */

#ifndef INC_PROTOCOLS_H_
#define INC_PROTOCOLS_H_

#define MAX_CHANNELS 10
#define INTERFRAME_MS 2
#define FRAME_TIMEOUT 100


typedef enum {
  INITIAL_INTERFRAME,
  FRAME
} ProtocolState;

extern uint16_t channels[MAX_CHANNELS];
extern uint32_t lastSentReportTime;
extern int uartInvert;
extern UART_HandleTypeDef huart2;
extern IWDG_HandleTypeDef hiwdg;

int ProtoWaitForInterframe(UART_HandleTypeDef* huart, uint32_t iframe, uint32_t timeout);
void BuildAndSendReport();
void ClearChannels();

void ProtoIbusReader(UART_HandleTypeDef*);
void ProtoIbusIa6Reader(UART_HandleTypeDef*);
void ProtoSbusReader(UART_HandleTypeDef*);
void DecodeSbusChannels(uint8_t *buf);
void ProtoDsmReader(UART_HandleTypeDef*);
void ProtoPpmReader(TIM_HandleTypeDef*);
void ProtoFportReader(UART_HandleTypeDef*);

void DebugLog(const char* buf);
void ResetWatchdog();

#endif /* INC_PROTOCOLS_H_ */
