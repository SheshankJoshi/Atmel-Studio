#include <atmel_start.h>
#include <lcd.h>
#include <util/delay.h>
#include <port.h>\

#define chosen_pin_RS 0
#define chosen_pin_En 1
char position=1;
void Lcd8_Setting(void);

int main(void)
{
	/* Initializes MCU, drivers and middle ware */
	atmel_start_init();
	Lcd8_Setting();
	Lcd8_Init();
	Lcd8_Write_String("Initializing done");
	_delay_ms(1000);
	Lcd8_Clear();
	//int i=0;
	/* Replace with your application code */
	while (1) {
		Lcd8_Write_String("This is a test Debug");
		_delay_ms(350);
		Lcd8_Clear();
		_delay_ms(100);
		//i++;
	}
}
void Lcd8_Setting(void) {
DDRD = 0xFF;
DDRB = 0xFF;
}
