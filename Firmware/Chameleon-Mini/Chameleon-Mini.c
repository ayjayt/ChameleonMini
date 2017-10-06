#include "Chameleon-Mini.h"
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/power.h>

void enableUSARTD1(void){
		PORTD.OUT |= PIN7_bm;
		PORTD.DIRSET = PIN7_bm;
		PORTD.DIRCLR = PIN6_bm;
		USARTD1.CTRLC = (uint8_t) USART_CHSIZE_8BIT_gc | (0b11<<4);
    USARTD1.BAUDCTRLA = (uint8_t) (3270);
    USARTD1.BAUDCTRLB = ( 0b1001 << USART_BSCALE0_bp ) | ( 3270>>8 );
		
	  USARTD1.CTRLB |= USART_RXEN_bm;
		USARTD1.CTRLB |= USART_TXEN_bm;
}

void ptc(char c){
	do{
			/* Wait until it is possible to put data into TX data register.*/
	} while(!( (USARTD1.STATUS & USART_DREIF_bm) != 0));
	USARTD1.DATA = c;
}
void pts(char * str, uint8_t len){
	uint8_t i = 0;
	if (len){
		for(; i < len; i++){
			ptc(str[i]);
		}
	} else {
		while (str[i] != 0){
			ptc(str[i++]);
		}
	}
}

char commandBuffer[10];
uint8_t commandCounter = 0;

int main(void)
{


	SystemInit(); // watchdog and oscillator stuff
	SettingsLoad(); // loads stuff from ee
	LEDInit(); // Sets direction Register
	MemoryInit(); //FRAM DMA stuff 
	CodecInitCommon(); // Does Pins for Modems
	ConfigurationInit(); // loads new configuration and finishes CodecInit
	//TerminalInit(); // sets port direction pon pind5 // senses usb, might be why clock turns back on
	RandomInit(); // empty function (literally 0 lines)
	ButtonInit(); // just some pullups and stuff
	AntennaLevelInit(); // sets ome things
	LogInit();
	SystemInterruptInit(); // Just enable all interrupts

	CLK.USBCTRL &= ~(1);
	USB.CTRLA &= ~(1<<7);
	SystemStopUSBClock();
	enableUSARTD1();
	
	
	volatile uint16_t sleeper = 0;
	while(1) {
		
		if (USARTD1.STATUS & USART_RXCIF_bm) {
			LEDHook(LED_POWERED, LED_ON);
			sleeper = 11;
			if (commandCounter == 10) commandCounter = 0;
			commandBuffer[commandCounter++] = USARTD1.DATA;

			if (commandBuffer[commandCounter-1] == 0b10101010) {
				pts("ok\0", 0);
				commandCounter = 0;
			} else {
				USARTD1.DATA = commandBuffer[commandCounter-1];
			}
		}

		if (SystemTick100ms()) {
			if (sleeper >= 1){
				sleeper--;
			} else {
				LEDHook(LED_POWERED, LED_OFF);
			}
			RandomTick(); // "entropy generator"
			//TerminalTick(); // PRETTY INVOLVED, ./Termina/* needs going through
			ButtonTick(); //Any results are stored in the settings structure (including button action function)
			LogTick(); //
			LEDTick(); //

			ApplicationTick(); // believe this is mifare related, CHECK
			//CommandLineTick();  // happens in TerminalTick too, doesn't look like it does much
			AntennaLevelTick(); //
		}
		//TerminalTask();
		LogTask();
		ApplicationTask();
		CodecTask();
	}
}

