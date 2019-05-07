#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include "datvexpressSink.h"

bool m_running;

DVBS2 m_dvbs2;
#define TSPLEN 188
//
// Signal handler
//
void my_handler(int s){
	m_running = false;
	express_receive();
	express_deinit();
    printf("Caught signal %d\n",s);
    exit(0); 
}

//
// Common print routine
//
void printcon( const char *fmt, ... )
{
    va_list ap;
    va_start(ap,fmt);
    vprintf(fmt,ap);
    va_end(ap);
}

void read_in_bytes( uint8_t *b, int len ){
    int to_read = len;
    int been_read = 0;
    while(to_read > 0 ){
        been_read += read(STDIN_FILENO,&b[been_read],to_read);
        to_read = len - been_read;
    }
}

void stdin_input(void){
	uint8_t buffer[TSPLEN];
    buffer[0] = 0;
   	while(m_running == true){
		read_in_bytes( buffer, TSPLEN );
   	    if(buffer[0] == 0x47){
			m_dvbs2.s2_add_ts_frame( buffer );
       	}else{
       	    // Not in sync so move on 1 byte
            read_in_bytes( buffer, 1 );
    	}
  	}
}
void udp_input(void){
	uint8_t buffer[TSPLEN];
    buffer[0] = 0;
   	while(m_running == true){
		get_udp_buffer( buffer, TSPLEN );
   	    if(buffer[0] == 0x47){
			m_dvbs2.s2_add_ts_frame( buffer );
       	}else{
       	    // Not in sync so move on 1 byte
			get_udp_buffer( buffer, 1 );
    	}
  	}
}
int main( int c, char *argv[]){
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

    if( express_init( "datvexpress16.ihx", "datvexpressraw16.rbf" ) == EXP_OK){
 		
		DVB2FrameFormat fmt;
        process_command_line( c, argv, &fmt);
   		if(m_dvbs2.s2_set_configure( &fmt ) == 0){
    		m_dvbs2.s2_register_tx(express_write_16_bit_samples);
            express_transmit();
			m_running = true;
			// Choose which forever loop
            if(is_udp_active() == true)
				udp_input();
			else
				stdin_input();
		}else{
			printcon( "Error: unable to configure S2 invalid parameter" );
		}
	}else{
		printcon( "Error: unable to init Express" );
	}
	return 0;
}
/*
static unsigned char m_null[TP_SIZE];

//
// Format a transport packet
//
void null_fmt( void )
{
    m_null[0] = 0x47;
    m_null[1] = 0;
    // Add the 13 bit pid
	m_null[1] = 0x1F;
	m_null[2] = 0xFF;
	m_null[3] = 0x10;
    for( int i = 4; i < TP_SIZE; i++ ) m_null[i] = 0xFF;
}

int main( int c, char *argv[]){
	uint8_t buffer[TSPLEN];
    buffer[0] = 0;
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	null_fmt();

    if( express_init( "datvexpress16.ihx", "datvexpressraw16.rbf" ) == EXP_OK){
 		
		DVB2FrameFormat fmt;
        process_command_line( c, argv, &fmt);
   		if(m_dvbs2.s2_set_configure( &fmt ) == 0){
    		m_dvbs2.s2_register_tx(express_write_16_bit_samples);
            express_transmit();
            m_running = true;
	    	while(m_running == true){
			    m_dvbs2.s2_add_ts_frame( m_null );
	    	}
		}else{
			printcon( "Error: unable to configure S2 invalid parameter" );
		}
	}else{
		printcon( "Error: unable to init Express" );
	}
	return 0;
}
*/
