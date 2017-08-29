/*
 * rf_rec.h
 *
 * Created: 28.08.2017 09:51:19
 *  Author: Simon_VBOX
 */ 


#ifndef RF_REC_H_
#define RF_REC_H_

#ifndef  F_CPU
	#define F_CPU 8000000
#endif
//#include <util/delay.h>

volatile uint8_t state;   //actual state of the receiver routine: SYNC, REC or READY
#define SYNC 1
#define REC 2
#define READY 3
uint32_t rf_code;		// received rf_code word. This is set when state switched to READY
volatile uint8_t buffer_of_flag;    // a flag which indicates a buffer-overflow of the timuings-buffer

//rf constants
#define _BUF		8		// Buffersize for timings. Should be at least 4
#define RF_MAX_BITS 32  //Maximum Number of Bits per transmission
#define RF_MIN_BITS 8	//Min number of bits per transmission#
#define SAMPLE_TIME 20  // Time for a sample in microseconds
#define RF_TOL 30 // Max toleranc of the sampletime in percent
#define RF_FAC		.9   //Sampletime-correction-factor due to code-runtime. 
// intertechno ptrotocol
// time is the duration of a tick in micros / SAMPLE_TIME. Note that time x longest_sequence<255, thus set the SAMPLE_TIME appropriate.   //TODO: may work with uint16
// sync1 and sync0 are the number of high and low ticks
// bit1 and bit0 is the sequence of signals eg bit1 is 1high 3low and bit0 is 3high 1 low in ticks
//			           Period    sync1, sync0,	bit11,12       bit01,02
#define PROTO0 {350/SAMPLE_TIME*RF_FAC,      1,    31,      1,3     ,      3,1      }
#define PROTO1 { 650/SAMPLE_TIME*RF_FAC,      1,10     ,     1,2 ,          2,1  }
#define PROTO2 { 100/SAMPLE_TIME*RF_FAC,      30,71    ,     4,11,          9,6  }
#define PROTO3 { 380/SAMPLE_TIME*RF_FAC,      1,6      ,     1,3 ,          3,1  }
#define PROTO4 { 500/SAMPLE_TIME*RF_FAC,      6,14     ,     1,2 ,          2,1  }
#define PROTO PROTO0
#define n_protos 5
uint8_t rf_active_proto;  // last recogniced protocol number

//interrupt vector for receiving a signal and the registers of the used timer.
#define RF_TIMER_VECT	TIMER0_COMPA_vect
#define RF_TIMSK		TIMSK0
#define RF_TCCRA		TCCR0A
#define RF_TCCRB		TCCR0B
#define RF_OCR			OCR0A
#define RF_OCIE			OCIE0A
#define RF_CTC_MODE		(1 << WGM01);

//the OCRVALUE to be used
#define RF_CLOCK_DIV (1 << CS01)  // set the clock prescaler to 8 	
#define RF_OCR_VAL (F_CPU/8/1000000*SAMPLE_TIME)  - 1 

//Port and Pin for receiving
#define RF_REC_PIN		(PINB & (1<<PINB3) )
#define RF_SETUP_PIN()   { 	DDRB&=~(1<<DDB3) ;  	PORTB |= (1 << PB3);  }

#define RF_SEND_PIN  2
#define RF_SEND_PORT PORTB
#define RF_SEND_DDR  DDRB


uint16_t get_timing(uint8_t i);
void rf_reset();
void rf_rec_init();
void rf_rec_stop();

void rf_send(uint32_t code, uint8_t len);
void rf_send_init();

uint32_t rf_get_code_from_tristate(uint32_t code);

#endif /* RF_REC_H_ */
