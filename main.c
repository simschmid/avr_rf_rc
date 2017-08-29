#define F_CPU 8000000


int main(void)
{
    /* Replace with your application code */
	
	uart_init();
	DDRD|=LED1|LED2;
	PORTD|=LED2;
	init_rf();
	char s[64];
	sprintf(s,"OCR0A=%d\n\r",OCR0A);
	UartPutS(s);
    while (1) 
    {
		
		//for (uint8_t i=0;i<5;i++)rf_send(0b100000100010,12);
		//_delay_ms(500);
		
		//wait for rf in
		//while(state!=READY);
		//_delay_ms(200);
		//PORTD^=LED1;

		while(Uartbufferlenth());
		//wait for bufferoverflow;
		//while(buffer_of_flag==0);
		//print_buffer();
		buffer_of_flag=0;

		//rf_reset();
		//while(state!=READY);
		
		
		if (state==READY){
			rf_reset();
			if (rf_code==0) continue;
		
			//sprintf(s,"ncstart=%d,ncstop=%d,delay=%d\n\r,",n_change_start,n_change_stop,sync_delay);
			//UartPutS(s);
		
		
			
			
			PORTD^=LED1;
			if (rf_get_code_from_tristate(rf_code) == 0b100000100010) {
				rf_rec_stop();
				for (uint8_t i=0;i<11;i++){
					PORTD^=LED2;
					_delay_ms(100);
				}
				
				rf_rec_init();
			}
			print_code(rf_code);
			while(Uartbufferlenth());
			print_code(rf_get_code_from_tristate(rf_code));
			while (Uartbufferlenth());
			sprintf(s,"Prot=%d",rf_active_proto);
			UartPutS(s);
			UartPutS("\n\r");
			//_delay_ms(500);
			
		}
    }
}
