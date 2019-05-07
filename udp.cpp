//
// This file contains the UDP handlers
//
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "datvexpressSink.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int        m_tx_sock;
int        m_rx_sock[2];
struct     sockaddr_in m_udp_cmd_client;
struct     sockaddr_in m_udp_cmd_server;
struct     sockaddr_in m_udp_trans_server;
socklen_t  m_udp_server_len;
extern int m_csock;
extern int m_tsock;

//
// This process reads UDP socket
//
/*
// First it initialises both receive and transmit sides
// Then it sits waiting to receive transport packets.
// It assumes the transport packets are correctly set up
// for the DVB transmit stream.
//
void *udp_proc( void * args)
{
    sys_config cfg;
    dvb_config_get( &cfg );

    args = args;

    if( cfg.tx_hardware == HW_UDP )
    {
        // Create the UDP socket
        if ((m_tx_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        {
            loggerf("Failed to create UDP TX socket\n");
            return 0;
        }

        // Construct the client sockaddr_in structure
        memset(&m_udp_client, 0, sizeof(m_udp_client));// Clear struct
        m_udp_client.sin_family = AF_INET;             // Internet/IP
        m_udp_client.sin_addr.s_addr = inet_addr(cfg.server_ip_address);  // IP address
        m_udp_client.sin_port = htons(cfg.server_socket_number);          // server port
    }

    if( cfg.capture_device_input == DVB_UDP )
    {
        struct sockaddr_in udp_server;
        int rx_sock;
        uchar buffer[MP_T_FRAME_LEN];
        socklen_t  udp_server_len = sizeof(udp_server);

        // Create the UDP socket
        if ((rx_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        {
            loggerf("Failed to create UDP RX socket\n");
            return 0;
        }

        // Construct the server sockaddr_in structure
        memset(&udp_server, 0, sizeof(udp_server));      // Clear struct
        udp_server.sin_family = AF_INET;                 // Internet/IP
        udp_server.sin_addr.s_addr = INADDR_ANY;         // IP address
        udp_server.sin_port = htons(cfg.server_socket_number); // server port

        bind(rx_sock, (struct sockaddr *)&udp_server, sizeof(udp_server));

        while( m_dvb_running )
        {
            int res = recvfrom( rx_sock, buffer, MP_T_FRAME_LEN, 0,
                         (struct sockaddr *) &udp_server, &udp_server_len);

            if( res == MP_T_FRAME_LEN )
            {
                // We have received a packet, check it is aligned
                if( buffer[0] == MP_T_SYNC)
                {
                    // Aligned packet queue for transmission
                    tx_write_transport_queue( buffer );
                }
                else
                {
                    // slip a byte, try to regain alignment
                    recvfrom( rx_sock, buffer, 1, 0,(struct sockaddr *) &udp_server, &udp_server_len);
                }
            }
        }
        loggerf("UDP process terminated\n");
    }
    return 0;
}

int udp_send_tp( uchar *buff )
{
    int res = sendto(m_tx_sock, buff, MP_T_FRAME_LEN, 0,
            (struct sockaddr *) &m_udp_client,
             sizeof(m_udp_client));

    return res;
}
*/
//
// This is because the Microsoft UDP interface acts on blocks
// and we want individual transport packets, so we have to buffer them
// I suspect Linux works in the same way.
//
static uint8_t m_udp_buffer[TP_SIZE*128];

int get_udp_buffer( uint8_t *b, int len )
{
    static int bytes_left;
    static int offset;

    if( bytes_left > len )
    {
        for( int i = 0; i < len; i++ ){
            b[i] = m_udp_buffer[offset++];
            bytes_left--;
        }
    }
    else
    {
        for( int i = 0; i < bytes_left; i++){
            b[i] = m_udp_buffer[offset++];
        }
        int start = bytes_left;
        // get a new buffer
        bytes_left = recvfrom( m_rx_sock[0], m_udp_buffer, TP_SIZE*128, 0,(struct sockaddr *) &m_udp_trans_server, &m_udp_server_len);
        offset     = 0;
        if(bytes_left > 0 )
        {
            for( int i = start; i < len; i++){
                b[i] = m_udp_buffer[offset++];
                bytes_left--;
            }
        }
        else
        {
            bytes_left = 0;
        }
    }
    return len;
}
//
//
//
//
int udp_read_transport( uint8_t *b, int length )

{
    return recvfrom( m_rx_sock[0], b, length, MSG_WAITALL,(struct sockaddr *) &m_udp_trans_server, &m_udp_server_len);
}
int udp_send_tp( uint8_t *buff, int length )
{
    return sendto(m_tx_sock, buff, length, 0, (struct sockaddr *) &m_udp_cmd_client, sizeof(m_udp_cmd_client));
}

//
// Initialise the UDP handler
//
int udp_init( int port )
{
    //socklen_t  m_udp_server_len = sizeof(m_udp_server);

    // Create the UDP socket
    if ((m_rx_sock[0] = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printcon("Failed create to RX transport socket\n");
        return -1;
    }

    // Construct the server sockaddr_in structure
    m_udp_server_len = sizeof(m_udp_trans_server);
    memset(&m_udp_trans_server, 0, sizeof(m_udp_trans_server));    // Clear struct
    m_udp_trans_server.sin_family      = AF_INET;                  // Internet/IP
    m_udp_trans_server.sin_addr.s_addr = INADDR_ANY;               // IP address
    m_udp_trans_server.sin_port        = htons(port);           // server port

    if( bind(m_rx_sock[0], (struct sockaddr *)&m_udp_trans_server, sizeof(m_udp_trans_server))< 0)
    {
        printcon("Failed to bind to RX transport socket\n");
        return -1;
    }
/*
    // Create the transmit socket for replies

    // Create the UDP socket
    if ((m_tx_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printcon("Failed to create UDP TX socket\n");
        return -1;
    }

    // Construct the client sockaddr_in structure
    memset(&m_udp_client, 0, sizeof(m_udp_client));// Clear struct
    m_udp_client.sin_family = AF_INET;             // Internet/IP
    m_udp_client.sin_addr.s_addr = inet_addr(cfg.server_ip_address);  // IP address
    m_udp_client.sin_port = htons(m_csock);          // server port
*/
    return 0;
}
