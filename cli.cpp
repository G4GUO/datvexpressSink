#include <string.h>
#include <stdlib.h>
#include "datvexpressSink.h"
#include "Express.h"

static bool m_udp_active;

void help(void){
	printf("-ft normal | short (frame type)\n");
	printf("-cr 1/4 | 1/3 | 2/5 | 1/2 | 3/5 | 2/3 | 3/4 | 4/5 | 5/6 | 8/9 | 9/10  (FEC rate)\n");
	printf("-ro 0.35 | 0.25 | 0.20  (filter roll off)\n");
	printf("-co qpsk | 8psk | 16apsk | 32 apsk (constellation)\n");
	printf("-pi on | off (pilots)\n");
	printf("-udp port number (UDP port)\n");
	printf("-fr transmit frequency in HZ (transmit frequency)\n");
	printf("-sr symbolrate  (symbol rate)\n");
	printf("-po power level (power level)\n");
	printf("-i  std | udp | null (where to get input)\n");
	printf("-help \n");
}
int parse(const char *a, const char *b, DVB2FrameFormat *fmt, SinkConfig *cfg ){

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
    // UDP 
    //
	if(strncmp(a,"-udp",4) == 0){
        int udp = atoi(b);
        udp_init( udp );
    	m_udp_active = true;
		return 2;
    } 
    //
	// Express specific
	// 
    if(strncmp(a,"-fr", 3) == 0 )
    {
        cfg->express_frequency = atof(b);
        return 2;
    }
    if(strncmp(a,"-sr", 3) == 0 )
    {
        cfg->express_symbolrate = atof(b);
        return 2;
    }
    if(strncmp(a,"-po", 3) == 0 )
    {
         cfg->express_level = atoi(b);
         return 2;
    }
    // What type of loop

    if(strncmp(a,"-i", 2) == 0 )
    {
        if(strncmp(b,"std", 3) == 0 )  cfg->input = STD_INPUT;
        if(strncmp(b,"udp", 3) == 0 )  cfg->input = UDP_INPUT;
        if(strncmp(b,"null", 4) == 0 ) cfg->input = NULL_INPUT;
        return 2;
    }
    if(strncmp(a,"-help", 5) == 0 )
    {
		help();
        exit(0);
    }
    printf("Uknown command %s\n",a);

	return 1;
}
bool is_udp_active(void){
	return m_udp_active;
}
int process_command_line( int c, char *argv[], DVB2FrameFormat *fmt, SinkConfig *cfg){
	// Set default S2 values
	fmt->frame_type = FRAME_NORMAL;
	fmt->code_rate = CR_2_3;
	fmt->roll_off = RO_0_20;
	fmt->constellation = M_QPSK;
	fmt->pilots = PILOTS_OFF;
	fmt->null_deletion = NPD_NOT_ACTIVE;
	fmt->coincedent_constellation = false;
    cfg->express_frequency   = 1255000000;
    cfg->express_symbolrate = 1000000;
    cfg->express_level = 20;
    m_udp_active = false;
    cfg->input = STD_INPUT;

	// Now read in command line
	int index = 1;
	while( index < c){
		index += parse(argv[index], argv[index+1], fmt, cfg );
    }
	return 0;
}
