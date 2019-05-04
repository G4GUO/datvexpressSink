#include <string.h>
#include <stdlib.h>
#include "DVBS2.h"
#include "Express.h"

int parse(const char *a, const char *b, DVB2FrameFormat *fmt ){

	if(strncmp(a,"-ft",3) == 0){
		if(strncmp(b,"normal",6)==0) fmt->frame_type = FRAME_NORMAL;
    	if(strncmp(b,"short",5)==0)  fmt->frame_type = FRAME_SHORT;
		return 2;
    } 
	if(strncmp(a,"-cr",3) == 0){
		if(strncmp(b,"1/4",3)==0)    fmt->code_rate  = CR_1_4;
		if(strncmp(b,"1/3",3)==0)    fmt->code_rate  = CR_1_3;
		if(strncmp(b,"2/5",3)==0)    fmt->code_rate  = CR_2_5;
		if(strncmp(b,"1/2",3)==0)    fmt->code_rate  = CR_1_2;
		if(strncmp(b,"3/5",3)==0)    fmt->code_rate  = CR_3_5;
		if(strncmp(b,"2/3",3)==0)    fmt->code_rate  = CR_2_3;
		if(strncmp(b,"3/4",3)==0)    fmt->code_rate  = CR_3_4;
		if(strncmp(b,"4/5",3)==0)    fmt->code_rate  = CR_4_5;
		if(strncmp(b,"5/6",3)==0)    fmt->code_rate  = CR_5_6;
		if(strncmp(b,"8/9",3)==0)    fmt->code_rate  = CR_8_9;
		if(strncmp(b,"9/10",4)==0)   fmt->code_rate  = CR_9_10;
		return 2;
    } 
	if(strncmp(a,"-ro",3) == 0){
		if(strncmp(b,"0.35",4)==0) fmt->roll_off = RO_0_35;
		if(strncmp(b,"0.25",4)==0) fmt->roll_off = RO_0_25;
		if(strncmp(b,"0.20",4)==0) fmt->roll_off = RO_0_20;
		return 2;
    } 
	if(strncmp(a,"-co",3) == 0){
		if(strncmp(b,"qpsk",4)==0)   fmt->constellation = M_QPSK;
		if(strncmp(b,"8psk",4)==0)   fmt->constellation = M_8PSK;
		if(strncmp(b,"16apsk",6)==0) fmt->constellation = M_16APSK;
		if(strncmp(b,"32apsk",6)==0) fmt->constellation = M_32APSK;
		return 2;
    } 
	if(strncmp(a,"-pi",3) == 0){
		if(strncmp(b,"on",2)==0)    fmt->pilots = PILOTS_ON;
		if(strncmp(b,"off",3)==0)   fmt->pilots = PILOTS_OFF;
		return 2;
    } 
    //
	// Express specific
	// 
    if(strncmp(a,"-fr", 3) == 0 )
    {
        double frequency = atof(b);
        express_set_freq( frequency );
        return 2;
    }
    if(strncmp(a,"-sr", 3) == 0 )
    {
        double symbol_rate = atof(b);
        express_set_sr( symbol_rate );
        return 2;
    }
	return 1;
}
int process_command_line( int c, char *argv[], DVB2FrameFormat *fmt){
	// Set default S2 values
	fmt->frame_type = FRAME_NORMAL;
	fmt->code_rate = CR_2_3;
	fmt->roll_off = RO_0_20;
	fmt->constellation = M_QPSK;
	fmt->pilots = PILOTS_OFF;
	fmt->null_deletion = NPD_NOT_ACTIVE;
	fmt->coincedent_constellation = false;
	// Now read in command line
	int index = 1;
	while( index < c){
		index += parse(argv[index], argv[index+1], fmt );
    }
	return 0;
}
