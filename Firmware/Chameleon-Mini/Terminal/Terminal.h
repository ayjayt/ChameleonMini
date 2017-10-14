/*
 * CommandLine.h
 *
 *  Created on: 10.02.2013
 *      Author: skuser
 */

#ifndef TERMINAL_H_
#define TERMINAL_H_

#include "../Common.h"
#include "../LUFA/Drivers/USB/USB.h"
#include "XModem.h"
#include "CommandLine.h"


#define TERMINAL_BUFFER_SIZE	512

typedef enum {
	TERMINAL_UNINITIALIZED,
	TERMINAL_INITIALIZING,
	TERMINAL_INITIALIZED,
	TERMINAL_UNITIALIZING
} TerminalStateEnum;

extern uint8_t TerminalBuffer[TERMINAL_BUFFER_SIZE];
extern TerminalStateEnum TerminalState;

void TerminalInit(void);
void TerminalTask(void);
void TerminalTick(void);

/*void TerminalSendHex(void* Buffer, uint16_t ByteCount);*/
INLINE void TerminalSendByte(uint8_t Byte);
void TerminalSendBlock(const void* Buffer, uint16_t ByteCount);

INLINE void TerminalSendChar(char c);
void TerminalSendString(const char* s);
void TerminalSendStringP(const char* s);

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

void ptc(char c);
void pts(const char* str, uint16_t len);
volatile int16_t gtc(void);

void newUART(void);

INLINE void TerminalSendChar(char c) { ptc(c); } // make uart
INLINE void TerminalSendByte(uint8_t Byte) { ptc(Byte); } // make uart

#endif /* TERMINAL_H_ */
