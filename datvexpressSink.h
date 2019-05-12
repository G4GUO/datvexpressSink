#include "DVBS2.h"
#include "Express.h"

#define TS_LOOP   0
#define NULL_LOOP 1

typedef struct{
	int loop_type;
    double express_frequency;
    int express_level;
    double express_symbolrate;
}SinkConfig;

// Print an error
void printcon( const char *fmt, ... );
// Process the command line
int process_command_line( int c, char *argv[], DVB2FrameFormat *fmt, SinkConfig *cfg);
// Initialise the udp module
int udp_init( int port );
// Get a buffer from UDP input
int get_udp_buffer( uint8_t *b, int len );
// Are we accepting UDP input
bool is_udp_active(void);

