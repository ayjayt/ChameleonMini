#include "Chameleon-Mini.h"
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/power.h>
volatile unsigned int sleepCounter = 0;

void enableUSARTD1(void){
		PORTD.OUT |= PIN7_bm;
		PORTD.DIRSET = PIN7_bm;
		PORTD.DIRCLR = PIN6_bm;
		USARTD1.CTRLC = (uint8_t) USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc | 0;
    USARTD1.BAUDCTRLA = (uint8_t) 33;
    USARTD1.BAUDCTRLB = ( 0b1111 << USART_BSCALE0_bp ) | ( 33 >> 8 );
	  USARTD1.CTRLB |= USART_RXEN_bm;
		USARTD1.CTRLB |= USART_TXEN_bm;
}

char trade(char sendData){
	do{
		/* Wait until it is possible to put data into TX data register.
		* NOTE: If TXDataRegister never becomes empty this will be a DEADLOCK. */
	} while(!( (USARTD1.STATUS & USART_DREIF_bm) != 0));
	USARTD1.DATA = sendData;

	uint16_t timeout = 20000;
	/* Receive one char. */
	do{
		/* Wait until data received or a timeout.*/
		timeout--;
	} while((!( (USARTD1.STATUS & USART_RXCIF_bm) != 0)) && timeout !=0);
	

	return USARTD1.DATA;
}

int main(void)
{


	SystemInit(); // watchdog and oscillator stuff
	SettingsLoad(); // loads stuff from ee
	LEDInit(); // Sets direction Register
	MemoryInit(); //FRAM DMA stuff 
	CodecInitCommon(); // Does Pins for Modems
	ConfigurationInit(); // loads new configuration and finishes CodecInit
	TerminalInit(); // sets port direction pon pind5 // senses usb, might be why clock turns back on
	RandomInit(); // empty function (literally 0 lines)
	ButtonInit(); // just some pullups and stuff
	AntennaLevelInit(); // sets ome things
	LogInit();
	SystemInterruptInit(); // Just enable all interrupts

	/*SystemStopUSBClock();
	//USBSEN to 0
	//CTRLA BIT 7 to 0
	CLK.USBCTRL &= ~(1);
	USB.CTRLA &= ~(1<<7);*/
	// What else can we do: change pins?
	while(1) {
	//CTRLA BIT 7 to 0
		/*SystemStopUSBClock();
		CLK.USBCTRL &= ~(1);
		USB.CTRLA &= ~(1<<7);*/
		if (SystemTick100ms()) {
			RandomTick(); // "entropy generator"
			TerminalTick(); // PRETTY INVOLVED, ./Termina/* needs going through
			ButtonTick(); //Any results are stored in the settings structure (including button action function)
			LogTick(); //
			LEDTick(); //
			ApplicationTick(); // believe this is mifare related, CHECK
			CommandLineTick();  // happens in TerminalTick too, doesn't look like it does much
			AntennaLevelTick(); //
			sleepCounter++; //
		}
		if (sleepCounter <= 25){
			LEDHook(LED_POWERED, LED_OFF);
		}
		if (sleepCounter >= 25){
			LEDHook(LED_POWERED, LED_ON);
		}
		if (sleepCounter == 50){
			sleepCounter = 0;
		}
		TerminalTask();
		LogTask();
		ApplicationTask();
		CodecTask();
	}
}

