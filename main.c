#define F_CPU 8000000


int main(void)
{
    /* Replace with your application code */
	
    rf_send_init();
    rf_rec_init();
    
    while (1) 
    {	
	// wait for a code and retranmsmit    
	while (state!=READY);
	rf_reset();
	if (rf_code==0x1e112){
		rf_send(rf_code);	
		;//do something
	}
    }
}
