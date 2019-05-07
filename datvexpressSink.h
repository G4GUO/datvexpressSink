#include "DVBS2.h"
#include "Express.h"
// Print an error
void printcon( const char *fmt, ... );
// Process the command line
int process_command_line( int c, char *argv[], DVB2FrameFormat *fmt);
// Initialise the udp module
int udp_init( int port );
// Get a buffer from UDP input
int get_udp_buffer( uint8_t *b, int len );
// Are we accepting UDP input
bool is_udp_active(void);

