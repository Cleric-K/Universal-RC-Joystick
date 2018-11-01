/*
 * protocols.h
 *
 *  Created on: 2.11.2018 ã.
 *      Author: Cleric
 */

#ifndef INC_PROTOCOLS_H_
#define INC_PROTOCOLS_H_

#define MAX_CHANNELS 10
#define INTERFRAME_MS 1
#define FRAME_TIMEOUT 100


typedef enum {
  INITIAL_INTERFRAME,
  FRAME
} ProtocolState;

extern uint16_t channels[MAX_CHANNELS];
extern uint32_t lastSentReportTime;
extern int uartInvert;

int ProtoWaitForInterframe(UART_HandleTypeDef* huart, uint32_t iframe, uint32_t timeout);
void BuildAndSendReport();
void ClearChannels();

void ProtoIbusReader(UART_HandleTypeDef*);
void ProtoIbusIa6Reader(UART_HandleTypeDef*);
void ProtoSbusReader(UART_HandleTypeDef*);
void ProtoDsmReader(UART_HandleTypeDef*);
void ProtoPpmReader(TIM_HandleTypeDef*);

#endif /* INC_PROTOCOLS_H_ */
