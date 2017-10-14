
#include "Terminal.h"
#include "../System.h"
#include "../LEDHook.h"


void ptc(char c){
	do{
			/* Wait until it is possible to put data into TX data register.*/
	} while(!( (USARTD1.STATUS & USART_DREIF_bm) != 0));
	USARTD1.DATA = c;
}
void pts(const char * str, uint16_t len){
	uint8_t i = 0;
	if (len){
		for(; i < len; i++){
			ptc(str[i]);
		}
	} else {
		while (str[i] != 0){
			ptc(str[i++]);
		} ptc(0);
	}
}

volatile int16_t gtc(){
	volatile int16_t recv;
	if (USARTD1.STATUS & USART_RXCIF_bm) {
		recv = USARTD1.DATA;
		ptc((char)recv);
		return (int16_t) recv;
	} else {
		return -1;
	}
}

#define INIT_DELAY		(2000 / SYSTEM_TICK_MS)


uint8_t TerminalBuffer[TERMINAL_BUFFER_SIZE];
TerminalStateEnum TerminalState = TERMINAL_UNINITIALIZED;


void TerminalSendString(const char* s) {
	pts(s, 0);
}

void TerminalSendStringP(const char* s) {
    char c;

    while( (c = pgm_read_byte(s++)) != '\0' ) {
        TerminalSendChar(c);
    }
}

/*
void TerminalSendHex(void* Buffer, uint16_t ByteCount)
{
    char* pTerminalBuffer = (char*) TerminalBuffer;

    BufferToHexString(pTerminalBuffer, sizeof(TerminalBuffer), Buffer, ByteCount);

    TerminalSendString(pTerminalBuffer);
}

*/


char bigBuffer[64];
uint16_t writeC = 0, readC = 0, mark = 0;
int16_t getByteFromBuffer(void){
	if (readC == 64){
		readC = 0;
	}
	if (mark){
		if (bigBuffer[readC] == '\r') {
			mark--;
		}
		return bigBuffer[readC++];
	}
	return -1;
}
void newUART(void){
	volatile int16_t rcvLocal = gtc();
	if (rcvLocal != -1){
		bigBuffer[writeC++] = (char)rcvLocal;
		if (writeC == 64) {
			writeC = 0;
		}
		if ((char)rcvLocal == '\r') {
			mark++;
		}
	}
}

void TerminalSendBlock(const void* Buffer, uint16_t ByteCount)
{
  	pts(Buffer, ByteCount); 
}


static void ProcessByte(void) {
		int16_t Byte = getByteFromBuffer();

    if (Byte >= 0) {
        /* Byte received */
    	LEDHook(LED_TERMINAL_RXTX, LED_PULSE);

        if (XModemProcessByte(Byte)) {
            /* XModem handled the byte */
        } else if (CommandLineProcessByte(Byte)) {
            /* CommandLine handled the byte */
        }
    }
}

static void SenseVBus(void)
{
		TerminalState = TERMINAL_INITIALIZED;
}

void TerminalInit(void)
{
   // Could put UART Init here 
}

void TerminalTask(void)
{
	if (TerminalState == TERMINAL_INITIALIZED) {

		// maybe these were used to actually send out what was in a buffer?

		ProcessByte();
	}
}

void TerminalTick(void)
{
	SenseVBus();

	if (TerminalState == TERMINAL_INITIALIZED) {
		XModemTick();
		CommandLineTick();
	}
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDHook(LED_TERMINAL_CONN, LED_ON);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDHook(LED_TERMINAL_CONN, LED_OFF);
}


/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
;
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
;
}


