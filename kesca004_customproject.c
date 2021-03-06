/*
 * kesca004_customproject.c
 *
 * Created: 8/22/2017 8:21:06 PM
 *  Author: KarenEscareno
 */ 


#include <avr/io.h>
#include "io.c"
#include <avr/interrupt.h>
const double init = 0.0;
const double c = 261.61;
const double d = 293.66;
const double e = 329.63;
const double f = 349.23;
const double g = 392.00;
const double a = 440.00;
const double b = 493.88;
const double cfive = 523.25;
double freq = 0.0;
unsigned char delilah = 0x00;
unsigned char mary = 0x00;
unsigned char barney = 0x00;
unsigned char song = 0x00;
unsigned char songs = 0x00;
unsigned char cursor = 1;
unsigned char led = 0x00;
unsigned char ledd = 0x00;
unsigned char twinkle = 0x00;
unsigned char row = 0x00;
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

enum SM_States {SM_Init, SM_Songlist, SM_Spider, SM_Songlists, SM_Jingle, SM_Mary, SM_Barney, SM_Twinkle, SM_Row, SM_Off} SM_State;

void PressButton()
{
	switch(SM_State) {
		
		case SM_Init:
		if((~PINA & 0x01) && !(~PINA & 0x02) && !(~PINA & 0x04) )
		{
			SM_State = SM_Songlist;
		}
		else if (!(~PINA & 0x01) && (~PINA & 0x02) && !(~PINA & 0x04))
		{
			SM_State = SM_Songlist;
		}
		else if (!(~PINA & 0x01) && !(~PINA & 0x02) && (~PINA & 0x04))
		{
			SM_State = SM_Songlist;
		}
		else if((~PINA & 0x08))
		{
			SM_State = SM_Songlist;
		}
		else if((~PINA & 0x10))
		{
			SM_State = SM_Songlist;
		}
		else{
			SM_State = SM_Init;
		}
		break;
		
		case SM_Songlist:
		if (song == 0x32)
		{
			if((~PINA & 0x01) && !(~PINA & 0x02) && !(~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					SM_State = SM_Jingle;
				}
				else {
					SM_State = SM_Songlist;
				}
			}
			else if (!(~PINA & 0x01) && (~PINA & 0x02) && !(~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					SM_State = SM_Mary;
				}
				else {
					SM_State = SM_Songlist;
				}
			}
			else if (!(~PINA & 0x01) && !(~PINA & 0x02) && (~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					SM_State = SM_Barney;
				}
				else {
					SM_State = SM_Songlist;
				}
			}
			else if (!(~PINA & 0x01) && !(~PINA & 0x02) && !(~PINA & 0x04))
			{
				if ((~PINA & 0x08) && !(~PINA & 0x10))
				{
					song = 0x00;
					SM_State = SM_Songlist;
				}
				if (!(~PINA & 0x08) && (~PINA & 0x10))
				{
					songs = 0x00;
					SM_State = SM_Songlists;
				}
				else {
					SM_State = SM_Songlist;
				}
			}
			else{
				SM_State = SM_Songlist;
			}
		}
		else {
			if((~PINA & 0x01) && !(~PINA & 0x02) && !(~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					SM_State = SM_Jingle;
				}
				else {
					SM_State = SM_Songlist;
				}
			}
			else if (!(~PINA & 0x01) && (~PINA & 0x02) && !(~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					SM_State = SM_Mary;
				}
				else 
				{
					SM_State = SM_Songlist;
				}
			}
			else if (!(~PINA & 0x01) && !(~PINA & 0x02) && (~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					SM_State = SM_Barney;
				}
				else 
				{
					SM_State = SM_Songlist;
				}
			}
			else if (!(~PINA & 0x01) && !(~PINA & 0x02) && !(~PINA & 0x04))
			{
				if ((~PINA & 0x08) && !(~PINA & 0x10))
				{
					song = 0x00;
					SM_State = SM_Songlist;
				}
				if (!(~PINA & 0x08) && (~PINA & 0x10))
				{
					songs = 0x00;
					SM_State = SM_Songlists;
				}
				else {
					SM_State = SM_Songlist;
				}
			}
			else{
				SM_State = SM_Songlist;
			}
		}
		break;
		
	case SM_Songlists:
		if (songs == 0x28)
		{
			if((~PINA & 0x01) && !(~PINA & 0x02) && !(~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					SM_State = SM_Twinkle;
				}
				else {
					SM_State = SM_Songlists;
				}
			}
			else if (!(~PINA & 0x01) && (~PINA & 0x02) && !(~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					SM_State = SM_Row;
				}
				else {
					SM_State = SM_Songlists;
				}
			}
			else if (!(~PINA & 0x01) && !(~PINA & 0x02) && (~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					songs = 0x00;
					SM_State = SM_Songlists;
				}
				else {
					SM_State = SM_Songlists;
				}
			}
			else if ((~PINA & 0x08) && !(~PINA & 0x10))
			{
				song = 0x00;
				SM_State = SM_Songlist;
			}
			else{
				SM_State = SM_Songlists;
			}
		}
		else {
			if((~PINA & 0x01) && !(~PINA & 0x02) && !(~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					SM_State = SM_Twinkle;
				}
				else 
				{
					SM_State = SM_Songlists;
				}
			}

			else if (!(~PINA & 0x01) && (~PINA & 0x02) && !(~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					SM_State = SM_Row;
				}
				else
				{
					SM_State = SM_Songlists;
				}
			}
			else if (!(~PINA & 0x01) && !(~PINA & 0x02) && (~PINA & 0x04))
			{
				if (!(~PINA & 0x08) && !(~PINA & 0x10))
				{
					songs = 0x00;
					SM_State = SM_Songlists;
				}
				else {
					SM_State = SM_Songlists;
				}
			}
			else if ((~PINA & 0x08) && !(~PINA & 0x10))
			{
				song = 0x00;
				SM_State = SM_Songlist;
			}	
			else{
				SM_State = SM_Songlists;
			}
		}
		break;
		
		case SM_Jingle:
		if ((~PINA & 0x08) && !(~PINA & 0x10))
		{
			SM_State = SM_Off;
		}
		else if ((~PINA & 0x10) && !(~PINA & 0x08))
		{
			SM_State = SM_Mary;
		}
		else if(delilah == 0x51)
		{
			SM_State = SM_Init;
		}
		else{
			SM_State = SM_Jingle;
		}
		break;
		
		case SM_Mary:
		if ((~PINA & 0x08) && !(~PINA & 0x10))
		{
			SM_State = SM_Off;
		}
		else if ((~PINA & 0x10) && !(~PINA & 0x08))
		{
			SM_State = SM_Barney;
		}
		else if(mary == 0x53)
		{
			SM_State = SM_Init;
		}
		else{
			SM_State = SM_Mary;
		}
		break;
		
		case SM_Barney:
		if ((~PINA & 0x08) && !(~PINA & 0x10))
		{
			SM_State = SM_Off;
		}
		else if ((~PINA & 0x10) && !(~PINA & 0x08))
		{
			SM_State = SM_Twinkle;
		}
		else if (barney == 0x64)
		{
			SM_State = SM_Init;
		}
		else{
			SM_State = SM_Barney;
		}
		break;
		
		case SM_Twinkle:
		if ((~PINA & 0x08) && !(~PINA & 0x10))
		{
			SM_State = SM_Off;
		}
		else if ((~PINA & 0x10) && !(~PINA & 0x08))
		{
			SM_State = SM_Row;
		}
		else if (twinkle == 0x58)
		{
			SM_State = SM_Init;
		}
		else{
			SM_State = SM_Twinkle;
		}
		break;
		
		case SM_Row:
		if ((~PINA & 0x08) && !(~PINA & 0x10))
		{
			SM_State = SM_Off;
		}
		else if ((~PINA & 0x10) && !(~PINA & 0x08))
		{
			SM_State = SM_Init;
		}
		else if (row == 0x20)
		{
			SM_State = SM_Init;
		}
		else{
			SM_State = SM_Row;
		}
		break;

		case SM_Off:
		SM_State = SM_Init;
		break;
		
		default:
		SM_State = SM_Init;
		break;
	}
	switch(SM_State) {
		case SM_Init:
		LCD_init();
		LCD_DisplayString(cursor,"Welcome. Press aButton to Start");
		set_PWM(0);
		PWM_on();
		PORTD = 0x00;
		PORTB = 0x00;
		delilah = 0x00;
		mary = 0x00;
		barney = 0x00;
		twinkle = 0x00;
		row = 0x00;
		song = 0x00;
		songs = 0x00;
		break;
		case SM_Songlist:
		if(song < 0x0A)
		{
			LCD_init();
			LCD_DisplayString(cursor,"1 - Jingle Bells");
			song = song + 0x01;
		}
		else if(song < 0x14)
		{
			LCD_init();
			LCD_DisplayString(cursor,"2 - Mary had a  Little Lamb");
			song = song + 0x01;
		}
		else if(song < 0x1E)
		{
			LCD_init();
			LCD_DisplayString(cursor,"3 - I Love You");
			song = song + 0x01;
		}
		else if(song < 0x28)
		{
			LCD_init();
			LCD_DisplayString(cursor,"4 - Repeat Song List");
			song = song + 0x01;
		}
		else if(song < 0x32)
		{
			LCD_init();
			LCD_DisplayString(cursor,"5 - Next Song   List");
			song = song + 0x01;
		}
		break;
		
		case SM_Songlists:
		if(songs < 0x0A)
		{
			LCD_init();
			LCD_DisplayString(cursor,"1 - Twinkle     Little Star");
			songs = songs + 0x01;
		}
		else if(songs < 0x14)
		{
			LCD_init();
			LCD_DisplayString(cursor,"2 - Row Row Row Your Boat");
			songs = songs + 0x01;
		}
		else if(songs < 0x1E)
		{
			LCD_init();
			LCD_DisplayString(cursor,"3 - Repeat Song List");
			songs = songs + 0x01;
		}
		else if(songs < 0x28)
		{
			LCD_init();
			LCD_DisplayString(cursor,"4 - Back Song   List");
			songs = songs + 0x01;
		}
		break;
		
		case SM_Jingle:
		LCD_init();
		LCD_DisplayString(1,"Jingle Bells");
		PORTB = 0x00;
		PORTD = 0x00;
		if(delilah < 0x02)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x03)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x05)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x06)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x08)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x0A)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x0C)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x0D)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x0F)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x10)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x12)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x14)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x16)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x17)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x19)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x1A)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x1C)
		{
			freq = c;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x1D)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x1F)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x20)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x22)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x24)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x26)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x27)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x29)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x2A)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x2C)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x2D)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x2F)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x30)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x32)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x33)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x35)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x36)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x38)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x3A)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x3C)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x3D)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x3F)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x40)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x42)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x43)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x45)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x46)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x48)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x49)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x4B)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x4C)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x4E)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x2A;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x4F)
		{
			set_PWM(0);
			PWM_on();
			delilah = delilah + 0x01;
		}
		else if(delilah < 0x51)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x15;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			delilah = delilah + 0x01;
		}
		break;
		
		case SM_Mary:
		LCD_init();
		LCD_DisplayString(1,"Mary Had A      Little Lamb");
		PORTB = 0x00;
		PORTD = 0x00;
		if(mary < 0x02)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x20;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x03)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x05)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x10;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x06)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x08)
		{
			freq = c;
			set_PWM(freq);
			led = (led & 0x00) | 0x08;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x09)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x0B)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x04;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x0C)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x0E)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x0F)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x11)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x01;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x12)
		{
			set_PWM(0);
			PWM_on();
			PORTD = 0x00;
			mary = mary + 0x01;
		}
		else if(mary < 0x14)
		{
			freq = e;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			mary = mary + 0x01;
		}
		else if(mary < 0x17)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x19)
		{
			freq = d;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			mary = mary + 0x01;
		}
		else if(mary < 0x1A)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x1C)
		{
			freq = d;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x01;
			PORTB = ledd;
			mary = mary + 0x01;
		}
		else if(mary < 0x1D)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x1F)
		{
			freq = d;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			mary = mary + 0x01;
		}
		else if(mary < 0x20)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x22)
		{
			freq = e;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			mary = mary + 0x01;
		}
		else if(mary < 0x23)
		{
			set_PWM(0);
			PWM_on();
			PORTB = 0x00;
			mary = mary + 0x01;
		}
		else if(mary < 0x25)
		{
			freq = e;
			set_PWM(freq);	
			led = (led & 0x00) | 0x01;
			PORTD = led;		
			mary = mary + 0x01;
		}
		else if(mary < 0x26)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		} 
		else if(mary < 0x28)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x2B)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x2D)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x04;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x2E)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x30)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x08;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x31)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x33)
		{
			freq = c;
			set_PWM(freq);
			led = (led & 0x00) | 0x10;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x34)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x36)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x20;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x37)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x39)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x10;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x3A)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x3C)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x08;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x3D)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x3F)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x04;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x42)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x44)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x45)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x47)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x01;
			PORTD = led;
			mary = mary + 0x01;
		}
		else if(mary < 0x48)
		{
			set_PWM(0);
			PWM_on();
			PORTD = 0x00;
			mary = mary + 0x01;
		}
		else if(mary < 0x4A)
		{
			freq = d;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			mary = mary + 0x01;
		}
		else if(mary < 0x4B)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x4D)
		{
			freq = e;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			mary = mary + 0x01;
		}
		else if(mary < 0x4E)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x50)
		{
			freq = d;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x01;
			PORTB = ledd;
			mary = mary + 0x01;
		}
		else if(mary < 0x51)
		{
			set_PWM(0);
			PWM_on();
			mary = mary + 0x01;
		}
		else if(mary < 0x53)
		{
			freq = c;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			mary = mary + 0x01;
		}
		break;
		
		case SM_Barney:
		LCD_init();
		LCD_DisplayString(1,"I Love You");
		PORTB = 0x00;
		PORTD = 0x00;
		if (barney < 0x02)
		{
			freq = g;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x03)
		{
			set_PWM(0);
			PWM_on();
			PORTB = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x05)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x05;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x06)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x08)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x28;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x0A)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x0C)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x14;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x0D)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x0F)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x10)
		{
			set_PWM(0);
			PWM_on();
			PORTD = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x12)
		{
			freq = g;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x14)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x16)
		{
			freq = a;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x17)
		{
			set_PWM(0);
			PWM_on();
			PORTB = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x19)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x05;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x1A)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x1C)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x28;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x1D)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x1F)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x14;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x20)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x22)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x23)
		{
			set_PWM(0);
			PWM_on();
			PORTD = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x25)
		{
			freq = e;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x26)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x28)
		{
			freq = f;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x2A)
		{
			set_PWM(0);
			PWM_on();
			PORTB = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x2C)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x05;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x2D)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x2F)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x28;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x30)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x32)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x14;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x33)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x35)
		{
			freq = c;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x36)
		{
			set_PWM(0);
			PWM_on();
			PORTD = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x38)
		{
			freq = c;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x3A)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x3C)
		{
			freq = c;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x3D)
		{
			set_PWM(0);
			PWM_on();
			PORTB = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x3F)
		{
			freq = c;
			set_PWM(freq);
			led = (led & 0x00) | 0x05;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x40)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x42)
		{
			freq = c;
			set_PWM(freq);
			led = (led & 0x00) | 0x28;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x43)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x45)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x14;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x46)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x48)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x49)
		{
			set_PWM(0);
			PWM_on();
			PORTD = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x4B)
		{
			freq = f;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x4C)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x4E)
		{
			freq = g;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x50)
		{
			set_PWM(0);
			PWM_on();
			PORTB = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x52)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x05;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x53)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x55)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x28;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x56)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x58)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x14;
			PORTD = led;
			barney = barney + 0x01;
		}
		else if (barney < 0x59)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x5B)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x5C)
		{
			set_PWM(0);
			PWM_on();
			PORTD = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x5E)
		{
			freq = e;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x5F)
		{
			set_PWM(0);
			PWM_on();
			barney = barney + 0x01;
		}
		else if (barney < 0x61)
		{
			freq = d;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x05;
			PORTB = ledd;
			barney = barney + 0x01;
		}
		else if (barney < 0x62)
		{
			set_PWM(0);
			PWM_on();
			PORTB = 0x00;
			barney = barney + 0x01;
		}
		else if (barney < 0x64)
		{
			freq = c;
			set_PWM(freq);
			led = (led & 0x00) | 0x05;
			PORTD = led;
			barney = barney + 0x01;
		}
		break;
		
		case SM_Twinkle:
		LCD_init();
		LCD_DisplayString(1,"Twinkle Twinkle Little Star");
		PORTB = 0x00;
		PORTD = 0x00;
		if(twinkle < 0x02)
		{
			freq = e;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x01;
			PORTB = ledd;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x03)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x05)
		{
			freq = e;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x06)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x08)
		{
			freq = b;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x09)
		{
			set_PWM(0);
			PWM_on();
			PORTB = 0x00;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x0B)
		{
			freq = b;
			set_PWM(freq);
			led = (led & 0x00) | 0x01;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x0C)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x0E)
		{
			freq = cfive;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x0F)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x11)
		{
			freq = cfive;
			set_PWM(freq);
			led = (led & 0x00) | 0x04;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x12)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x14)
		{
			freq = b;
			set_PWM(freq);
			led = (led & 0x00) | 0x08;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x16)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x18)
		{
			freq = a;
			set_PWM(freq);
			led = (led & 0x00) | 0x10;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x19)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x1B)
		{
			freq = a;
			set_PWM(freq);
			led = (led & 0x00) | 0x20;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x1C)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x1E)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x10;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x1F)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x21)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x08;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x22)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x24)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x04;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x25)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x27)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x28)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x2A)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x01;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x2C)
		{
			set_PWM(0);
			PWM_on();
			PORTD = 0x00;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x2E)
		{
			freq = b;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x2F)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x31)
		{
			freq = b;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x32)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x34)
		{
			freq = a;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x01;
			PORTB = ledd;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x35)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x37)
		{
			freq = a;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x02;
			PORTB = ledd;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x38)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x3A)
		{
			freq = g;
			set_PWM(freq);
			ledd = (ledd & 0x00) | 0x04;
			PORTB = ledd;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x3B)
		{
			set_PWM(0);
			PWM_on();
			PORTB = 0x00;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x3D)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x01;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x3E)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x40)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x02;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x42)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x44)
		{
			freq = b;
			set_PWM(freq);
			led = (led & 0x00) | 0x04;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x45)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x47)
		{
			freq = b;
			set_PWM(freq);
			led = (led & 0x00) | 0x08;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x48)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x4A)
		{
			freq = a;
			set_PWM(freq);
			led = (led & 0x00) | 0x10;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x4B)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x4D)
		{
			freq = a;
			set_PWM(freq);
			led = (led & 0x00) | 0x20;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x4E)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x50)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x10;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x51)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x53)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x08;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x54)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x56)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x04;
			PORTD = led;
			twinkle = twinkle + 0x01;
		}
		else if(twinkle < 0x58)
		{
			set_PWM(0);
			PWM_on();
			twinkle = twinkle + 0x01;
		}
		break;

		case SM_Row:
		LCD_init();
		LCD_DisplayString(1,"Row Row Row YourBoat");
		PORTB = 0x00;
		PORTD = 0x00;
		if(row < 0x02)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x3F;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x07;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x03)
		{
			set_PWM(0);
			PWM_on();
			led = (led & 0x00) | 0x00;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x00;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x05)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x3F;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x07;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x06)
		{
			set_PWM(0);
			PWM_on();
			led = (led & 0x00) | 0x00;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x00;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x08)
		{
			freq = d;
			set_PWM(freq);
			led = (led & 0x00) | 0x3F;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x07;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x09)
		{
			set_PWM(0);
			PWM_on();
			led = (led & 0x00) | 0x00;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x00;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x0B)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x3F;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x07;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x0C)
		{
			set_PWM(0);
			PWM_on();
			led = (led & 0x00) | 0x00;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x00;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x0E)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x3F;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x07;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x10)
		{
			set_PWM(0);
			PWM_on();
			led = (led & 0x00) | 0x00;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x00;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x12)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x3F;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x07;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x13)
		{
			set_PWM(0);
			PWM_on();
			led = (led & 0x00) | 0x00;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x00;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x15)
		{
			freq = e;
			set_PWM(freq);
			led = (led & 0x00) | 0x3F;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x07;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x16)
		{
			set_PWM(0);
			PWM_on();
			led = (led & 0x00) | 0x00;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x00;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x18)
		{
			freq = f;
			set_PWM(freq);
			led = (led & 0x00) | 0x3F;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x07;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x19)
		{
			set_PWM(0);
			PWM_on();
			led = (led & 0x00) | 0x00;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x00;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x1B)
		{
			freq = g;
			set_PWM(freq);
			led = (led & 0x00) | 0x3F;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x07;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x1C)
		{
			set_PWM(0);
			PWM_on();
			led = (led & 0x00) | 0x00;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x00;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x1E)
		{
			freq = a;
			set_PWM(freq);
			led = (led & 0x00) | 0x3F;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x07;
			PORTB = ledd;
			row = row + 0x01;
		}
		else if(row < 0x20)
		{
			set_PWM(0);
			PWM_on();
			led = (led & 0x00) | 0x00;
			PORTD = led;
			ledd = (ledd & 0x00) | 0x00;
			PORTB = ledd;
			row = row + 0x01;
		}
		break;
		
		case SM_Off:
		set_PWM(0);
		PWM_on();
		break;
		default:
		break;
	}
 
}


int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	
	TimerSet(1000);
	TimerOn();
	PWM_on();
	SM_State = SM_Init;
	LCD_init();
	while(1) {
	PressButton();
	while (!TimerFlag)
	TimerFlag = 0;
	} // while (1)
	
} // Main