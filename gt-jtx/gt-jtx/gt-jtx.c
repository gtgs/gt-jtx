/*
 * gt-jtx.cpp
 *
 * Created: 20-09-2012 15:08:33
 *  Author: Girish Sarwal
 *	 Email: g.sarwal@gmail.com
 *
 */
#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <math.h>
#include <util/delay.h>
#include <avr/interrupt.h>


void setup();
uint16_t map(uint16_t);
void reset();
uint16_t readAnalog(uint8_t);
void applyTrimTransforms();
void applyEndPointTransforms();

#define SYNC 0
#define AIL	1
#define ELE	2
#define THR	3
#define RUD	4
#define FLAPS 5
#define GEAR 6
#define MIX 7
#define AUX 8
#define MAX_CHANNEL AUX

#define MIN_SIGNAL_WIDTH 200
#define MAX_SIGNAL_WIDTH 2000
#define SERVO_MIN_SIGNAL 500
#define SERVO_MAX_SIGNAL 1500
#define SERVO_TRAVERSAL SERVO_MAX_SIGNAL - SERVO_MIN_SIGNAL


#define STARTADC ADCSRA |= (1<<ADEN) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADIE);

uint16_t ppm[MAX_CHANNEL];
static int channel = SYNC;

int main(){
	reset();
	
	setup();
	
	/**Enable global interrupts **/
	sei();

	/** start our timer **/
	TIMSK |= (1<<OCIE1A);
	/** start the adc **/
	ADCSRA |= (1<<ADSC);
	while(1){
		/**read inputs **/
		ppm[AIL] = map(readAnalog(AIL));
		ppm[ELE] = map(readAnalog(ELE));
		ppm[THR] = map(readAnalog(THR));
		}
};

uint16_t map(uint16_t value){
	float scaleValue = value/1024.0f;
	return (500 + scaleValue * 1000);
}

ISR(TIMER1_COMPA_vect){
	TIMSK &= ~(1<<OCIE1A);
	
	if(channel >= MAX_CHANNEL){
		channel = -1;
	}
	
	if((PINB & 0x02) == 0x02)		//If the actual pin is high, we need to set OCR1A to the complementary delay
		OCR1A = ppm[++channel];		
	else
		OCR1A = MAX_SIGNAL_WIDTH - ppm[channel];	

	TIMSK |= (1<<OCIE1A);
};

void setup(){
	/** set output **/
	DDRB = 0x02;						/** make PB1 as out pin **/
	
	PORTB = 0x02;						/** make PB1 as high **/
	TCNT1 = 0;
	
	TCCR1B |= (1<<CS10) ;				/** F_CPU - 1Mhz Timebase **/
	TCCR1B	|=	(1<<WGM12);				/** CTC Mode**/
	OCR1A = ppm[SYNC];	
	TCCR1A |= (1<<COM1A0);				/** hardware ctc **/
	
	/** set inputs **/					/** PORTC in the input for ADC control on ATmega8**/
	PORTC = 0x00;
	ADMUX |= (1<<REFS0);				
	ADCSRA |= (1<<ADEN); 				/** enable the ADC **/
	ADCSRA|= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS0);	/**F_CPU/128 Prescalar **/
	
};
uint16_t readAnalog(uint8_t ch){
	ch = ch & 0x07;			/** zero out all bits except last three for limiting channel **/
	ADMUX &=0b11100000;		/**reset channel selection **/
	ADMUX |=ch;						/**select the requested channel **/
	ADCSRA |= (1<<ADSC);
	while(!(ADCSRA & (1<<ADIF)));
	ADCSRA|= (1<<ADIF);
	if(ADC <= MIN_SIGNAL_WIDTH)
		return MIN_SIGNAL_WIDTH;
	return (ADC);
};
void reset(){
	/** set values to match servo spec **/
	ppm[SYNC] = 1000;
	ppm[AIL] = 1000;
	ppm[ELE] = 1000;
	ppm[THR] = 1000;
	ppm[RUD] = 1000;
	ppm[FLAPS] = 1000;
	ppm[GEAR] = 1000;
	ppm[MIX] = 1000;
	ppm[AUX] = 1000;
};





