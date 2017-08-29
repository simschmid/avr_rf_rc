/*
 * rf_rec.c
 *
 * Created: 28.08.2017 09:51:04
 *  Author: Simon_VBOX
 */ 


#include <avr/io.h>
#include <avr/pgmspace.h>
//#define F_CPU 8000000


#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "rf_rec.h"
#include <util/delay.h>



typedef struct {
	uint16_t time;
	uint8_t sync1;
	uint8_t sync0;
	
	uint8_t bit11;
	uint8_t bit10;
	
	uint8_t bit01;
	uint8_t bit00;
	
} Proto;


//set Protocol
Proto prot = PROTO;

const Proto protos[]={PROTO0,PROTO1,PROTO2,PROTO3,PROTO4};


void init_rf();
uint16_t timings[_BUF];   //stores the timings of a signal
uint8_t n_change=0;		// counter

uint16_t time=0;		// time counter
volatile uint8_t buffer_of_flag=0;  // a flag which indicates a buffer overflow of the timings array


// Definitions for the reception states



// The findal code registers
uint32_t _rf_code=0;
//uint32_t rf_code=0;

uint8_t last_=0; //the last pin state. Neded to detect a change in signal-level

// function for recording a transmission
void rf_rec();
//reset states and flags
void rf_reset();


// Interrupt-routine for receiving rf-timings
ISR(RF_TIMER_VECT){
	
	uint8_t val=RF_REC_PIN;

	//Check for changed bit and save in timingsbuffer
	if(last_!=val){
		timings[n_change]=time;
		rf_rec();
		time=0;
		n_change++;
		//check for oveflow
		if (n_change>=_BUF) {
			n_change=0;
			buffer_of_flag=1; //set overflow flag
		}
		
	}
	last_=val;
	
	time++;
}

void rf_reset(){
	state=SYNC;
}

//helperfunction abs(a-b)
uint16_t diff(uint16_t a, uint16_t b){
	if (b>a) return b-a;
	return a-b;
}


/**
 * check if abs(a-b)<=b*RF_TOL/100
 * 
 * \param a
 * \param b
 * 
 * \return uint8_t
 */
uint8_t in_tol(uint16_t a, uint16_t b){
	return diff(a,b)<=(((uint32_t) b*RF_TOL * 5)>>9)+1 ? 1 :0;
}
/*
Get the i-th last saved timing from buffer
*/
uint16_t get_timing(uint8_t i){
	
	if (i>n_change) return timings[ _BUF-i+n_change];
	return timings[ n_change-i];
	
}

//uint8_t bit_buffer=0;


/**
 * Handlerfunction after receiving a new timing
 * 
 * 
 * \return void
 */
void rf_rec() {
	static uint8_t n_bits=0;
	static uint8_t bit_buffer_idx=0;
	static uint16_t time=0;
	//static Proto prot=PROTO0;
	if (state==READY) return ;
	//check for sync
	
	if ( state!=REC && get_timing(0)*SAMPLE_TIME>2000 ){
		for (uint8_t i=0;i<n_protos;i++){
			prot=protos[i];
			if(in_tol(get_timing(0),prot.time*prot.sync0)  &&  in_tol(get_timing(1),prot.time*prot.sync1) ){
				//start recording a signal
				state=REC;
				bit_buffer_idx=0;
				_rf_code=0;
				n_bits=0;
				time=prot.time;// get_timing(0)/prot.sync0;
				rf_active_proto=i;
				//n_change_start=n_change;
				return;
			}

		}
	}
	else if (state == REC ){
		//need two timings to construct a bit
		if(bit_buffer_idx==1 ){
			_rf_code<<=1;
			if (  in_tol(get_timing(1),time*prot.bit11)  && in_tol(get_timing(0),time*prot.bit10)){  _rf_code |=1;}  // a one bit
			else if (  in_tol(get_timing(1),time*prot.bit01)  && in_tol(get_timing(0),time*prot.bit00) ) { ;}  // zero
			else {state=READY;}
				
				
			n_bits+=1;
			if (n_bits>=RF_MAX_BITS) state=READY;
				
			if (state==READY){
				if (n_bits <RF_MIN_BITS) state=SYNC;
				rf_code=_rf_code>>1;				
			}
			

			bit_buffer_idx=0;
		}
		else bit_buffer_idx++;
	}
}






void rf_rec_old() {
	static uint8_t n_bits=0;
	static uint8_t bit_buffer_idx=0;
	static uint16_t time=0;
	static uint8_t n_repeat=0;
	//static Proto prot=PROTO0;
	if (state==READY) return ;
	//check for sync
	
	if ( state!=REC && get_timing(0)*SAMPLE_TIME>2000 ){
		n_repeat++;
		if(n_repeat==2){
			n_repeat=0;
			for (uint8_t i=0;i<n_protos;i++){
				prot=protos[i];
				if(in_tol(get_timing(0),prot.time*prot.sync0)  &&  in_tol(get_timing(1),prot.time*prot.sync1) ){
					//start recording a signal
					state=REC;
					bit_buffer_idx=0;
					_rf_code=0;
					n_bits=0;
					time=prot.time;// get_timing(0)/prot.sync0;
					rf_active_proto=i;
					//n_change_start=n_change;
					return;
				}
			}
		}
	}
	else if (state == REC ){
		//need two timings to construct a bit
		if(bit_buffer_idx==_BUF-1 || get_timing(0)*SAMPLE_TIME>2000){
			for (uint8_t j=0;j<=bit_buffer_idx;j+=2){
				uint8_t i=bit_buffer_idx-j-1;
				//a bit has been recorded
				_rf_code<<=1;
							
				if (  in_tol(get_timing(1+i),time*prot.bit11)  && in_tol(get_timing(0+i),time*prot.bit10)){  _rf_code |=1;}  // a one bit
				else if (  in_tol(get_timing(1+i),time*prot.bit01)  && in_tol(get_timing(0+i),time*prot.bit00) ) { ;}  // zero
				else {state=READY;}
							
							
				n_bits+=1;
				if (n_bits>=RF_MAX_BITS) state=READY;
							
				if (state==READY){
					if (n_bits <RF_MIN_BITS) state=SYNC;
					rf_code=_rf_code>>1;
					break;
				}
			} 

			bit_buffer_idx=0;
		}
		else bit_buffer_idx++;
	}
}

/************************************************************************/
/* Initiates receiver interrupts                                                                     */
/************************************************************************/
void rf_rec_init(){
	RF_SETUP_PIN();
	RF_TCCRA |= RF_CTC_MODE;		//CTC Mode
	RF_TCCRB |= RF_CLOCK_DIV;		//clk div=8
	RF_TIMSK |= (1 << RF_OCIE);		//Enable Compare Match A interrupt
	
	RF_OCR=RF_OCR_VAL;
}

/**
 * Disable receiver interrupt 
 * 
 * \return void
 */
void rf_rec_stop(){
	RF_TIMSK &= ~ (1<< RF_OCIE);
}

/**
 * generate a delay of ticks x SAMPLE_TIME
 * 
 * \param ticks
 * 
 * \return void
 */
void delay(uint16_t ticks){
	ticks=ticks*(prot.time);
	while(ticks>0){
		 _delay_us(SAMPLE_TIME);
		ticks--;	 
	}
}


//Toggle RF_SEMD_PIN to high
#define RF_HI() {RF_SEND_PORT|=(1<<RF_SEND_PIN);}
//Toggle RF_SEMD_PIN to low
#define RF_LO() {RF_SEND_PORT&=~(1<<RF_SEND_PIN);}
void send_sync(){
	// hi 
	RF_HI();
	delay( prot.sync1 );
	RF_LO();
	delay( prot.sync0);
}


/*send a single bit
 *\param bit: 0 or 1
*/
void send_bit(uint8_t bit){
	RF_HI();
	delay(bit>0?prot.bit11:prot.bit01);
	RF_LO();
	delay(bit>0?prot.bit10:prot.bit00);
}


/**
 * Send a code via RF_SEND_PIN
 * 
 * \param code: a 32bit Word
 * \param len: number of bits to transmit starting from LSB
 * 
 * \return void
 */
void rf_send(uint32_t code,const uint8_t len){
	// disable receiver
	rf_rec_stop();
	//cli();
	//send sync
	send_sync();
	//send code
	
	for (uint8_t i=0; i<len;i++){
		if (code & (1<<(len-1-i))) send_bit(1);
		else send_bit(0);
		//code<<=1;
	}
	
	RF_LO();
	//restart receiver
	rf_rec_init();
	//sei();
	
}

/**
 * Initiate the RF_SEND_PIN
 * 
 * 
 * \return void
 */
void rf_send_init(){
	
	DDRB|=(1<<RF_SEND_PIN);
	RF_LO();
}



/**
 * Convert a tristate bit pattern to 32bit word
 * 
 * \param code
 * 
 * \return uint32_t
 */
uint32_t rf_get_code_from_tristate(uint32_t code){
	uint32_t ret_code=0;
	for (uint8_t i=0;i<32;i+=2){
		if (code&(1L<<i)) ret_code|=(1L<<(i/2));
	}
	return ret_code;
}


/**
 * Convert and send a 32bit word as tristate bit pattern
 * 
 * \param code
 * \param len: number of bits to send, starting from LSB
 * 
 * \return void
 */
void rf_send_tristate(uint32_t code,uint8_t len){
	//firsst convert
	uint32_t ret_code=0;
	for (uint8_t i=0;i<len;i++){
		if (code &(1L<<i)) ret_code|=(1L<<(2*i));
		ret_code|=(1L<<(2*i+1));
	}
	rf_send(ret_code,2*len);
}
