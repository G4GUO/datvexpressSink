#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include "DVBS2.h"
#include "Express.h"
#include "datvexpressSink.h"

DVBS2 m_dvbs2;
#define TSPLEN 188
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

int main( int c, char *argv[]){
	uint8_t buffer[TSPLEN];

    if( express_init( "datvexpress16.ihx", "datvexpressraw16.rbf" ) == EXP_OK){
 		
		DVB2FrameFormat fmt;
        process_command_line( c, argv, &fmt);
   		if(m_dvbs2.s2_set_configure( &fmt ) == 0){
    		m_dvbs2.s2_register_tx(express_write_16_bit_samples);

	    	while(1){
		    	if(fread(buffer,TSPLEN,1,stdin) == TSPLEN){
            	    if(buffer[0] == 0x47){
						m_dvbs2.s2_add_ts_frame( buffer );
            	    }else{
            		    // Not sync so move on 1 byte
        			    fread(buffer,1,1,stdin);
            	    }
		    	}
	    	}
		}else{
			printcon( "Error: unable to configure S2 invalid parameter" );
		}
		express_deinit();
	}else{
		printcon( "Error: unable to init Express" );
	}
	return 0;
}
