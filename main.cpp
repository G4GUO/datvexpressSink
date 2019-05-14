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
#define TSPLEN  188
#define TP_SYNC 0x47
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
static unsigned char m_null[TP_SIZE];

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
//
// increment the counter in a transport packet
//
void inc_seq_count(uint8_t *b){
	uint8_t c = b[3]&0x0F;
    c = (c+1)&0x0F;
    b[3] = (b[3]&0xF0)|c;
}
//
// This processes the transport stream from the UDP socket
//
void *udp_thread( void *arg )
{
    // This blocks
    uint8_t *b;

    while( m_running )
    {
        b = alloc_buff();
        get_udp_buffer( b, TSPLEN );
        if( b[0] != TP_SYNC ){ // Try to achieve sync
            for( int i = 0; i < TSPLEN-1; i++ ){
                get_udp_buffer( b, 1 );
                if( b[0] == TP_SYNC ){
                    get_udp_buffer( b, TSPLEN - 1 );
                }
            }
        }
        // Queue
        post_buff( b );
    }
    return arg;
}
//
// This processes the transport stream from STDIN
//
void *stdin_thread( void *arg )
{
    // This blocks
    uint8_t *b;

    while( m_running )
    {
        b = alloc_buff();
        int len = fread(b, 1, TSPLEN, stdin);
        if( b[0] == TP_SYNC )
        {
            // Aligned to TP (probably)
            post_buff( b );
        }
        else
        {
            // Slip bytes until TP alignment
            printcon("STDIN Invalid sync byte %.2X %d\n",b[0],len);
            len = fread(b, 1, 1, stdin);
            rel_buff( b );
        }
    }
    return arg;
}
//
// Feed modulator with transport packets
//

void ts_loop(void){
	uint8_t *b;
	while( m_running == true){
        if((b=get_buff())!= NULL)
	        m_dvbs2.s2_add_ts_frame( b );
         else
        {
           // Underrun
	       m_dvbs2.s2_add_ts_frame( m_null );
        }
   	}
}

static pthread_t m_thread;

void std_loop(void){
	// Start the std input loop thread
    if(pthread_create( &m_thread, NULL, stdin_thread, NULL ) != 0 )
    {
        printcon("Unable to start stdin thread\n");
        m_running = false;
        return;
    }
	ts_loop();
}

void udp_loop(void){
	// Start the udp loop thread
    if(pthread_create( &m_thread, NULL, udp_thread, NULL ) != 0 )
    {
        printcon("Unable to start udp thread\n");
        m_running = false;
        return;
    }
	ts_loop();
}

void null_loop(void){
    // Just send NULL packets 
	while( m_running == true){
		inc_seq_count( m_null);
	    m_dvbs2.s2_add_ts_frame( m_null );
   	}
}

int main( int c, char *argv[]){
	struct sigaction sigIntHandler;
    SinkConfig cfg;
	DVB2FrameFormat fmt;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	buf_init();

    process_command_line( c, argv, &fmt, &cfg);

    if( express_init( "datvexpress16.ihx", "datvexpressraw16.rbf" ) == EXP_OK){
        express_set_freq( cfg.express_frequency );
        express_set_sr( cfg.express_symbolrate );
        express_set_level( cfg.express_level );
 		
   		if(m_dvbs2.s2_set_configure( &fmt ) == 0){
    		m_dvbs2.s2_register_tx(express_write_16_bit_samples);
            printcon("Channel bitrate %d\n",(int)(m_dvbs2.s2_get_efficiency()*cfg.express_symbolrate));
            express_transmit();
			m_running = true;
 			null_fmt();

           if(cfg.input == STD_INPUT ) std_loop();
           if(cfg.input == UDP_INPUT ) udp_loop();
           if(cfg.input == NULL_INPUT ) null_loop();
		}else{
			printcon( "Error: unable to configure S2 invalid parameter" );
		}
	}else{
		printcon( "Error: unable to init Express" );
	}
	return 0;
}

