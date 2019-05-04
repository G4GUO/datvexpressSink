#include "DVB2.h"
#include "memory.h"
//
// This file adds the BB header and new transport packets
//
#define CRC_POLY 0xAB
// Reversed
#define CRC_POLYR 0xD5

void DVB2::dvb2_build_crc8_table( void )
{
    int r,crc;

    for( int i = 0; i < 256; i++ )
    {
        r = i;
        crc = 0;
        for( int j = 7; j >= 0; j-- )
        {
            if((r&(1<<j)?1:0) ^ ((crc&0x80)?1:0))
                crc = (crc<<1)^CRC_POLYR;
            else
                crc <<= 1;
        }
        m_crc_tab[i] = crc;
    }
}

uint8_t DVB2::dvb2_calc_crc8( uint8_t *b, int len )
{
    uint8_t crc = 0;

    for( int i = 0; i < len; i++ )
    {
        crc = m_crc_tab[b[i]^crc];
    }
    return crc;
}

int DVB2::dvb2_add_bbheader_bytes(void)
{
	uint16_t temp;
	BBHeader *h = &m_format.bb_header;
	uint8_t *m = m_frame_bytes;
	m[0] = h->ts_gs;
	m[0] <<= 1;
	m[0] |= h->sis_mis;
	m[0] <<= 1;
	m[0] |= h->ccm_acm;
	m[0] <<= 1;
	m[0] |= h->issyi;
	m[0] <<= 1;
	m[0] |= h->npd;
	m[0] <<= 2;
	m[0] |= h->ro;

	m[1] = h->sis_mis == SIS_MIS_MULTIPLE ? h->isi : 0;

	m[2] = h->upl >> 8;
	m[3] = h->upl & 0xFF;

	m[4] = h->dfl >> 8;
	m[5] = h->dfl & 0xFF;

	m[6] = h->sync;

	// Calculate syncd, this should point to the MSB of the CRC
	// use the overflow from the lat frame.

	temp = m_tp_overflow;
	if (temp == 0)
		temp = 187 * 8;
	else
		temp = (temp - 1) * 8;
	m[7] = temp >> 8;
	m[8] = temp & 0xFF;

	// Add CRC to BB header, at end
	m[9] = dvb2_calc_crc8( m, 9);

	// Always 10 bytes / 80 bits long
	return 10;
}

//
// Formatted into a binary array
//
//
// length is in bytes
//
int DVB2::dvb2_unpack_bytes(uint8_t *in, Bit *out, int len) {
	int index = 0;
	for (int i = 0; i < len; i++) {
		for (int n = 7; n >= 0; n--)
		{
			out[index++] = (in[i]&(1 << n)) ? 1 : 0;
		}
	}
	return index;
}

void DVB2::dvb2_add_crc_to_tp_and_queue(uint8_t *ts)
{
	// CRC is added in place of sync byte at end of packet
	// skip the header 0x47
	uint8_t crc = dvb2_calc_crc8(&ts[1], 188 - 1);
	// Add the transport packet to the transport queue
	for (int i = 1; i < 188; i++) m_tp_q.push(ts[i]);
	// Add the crc
	m_tp_q.push(crc);
}
bool DVB2::dvb2_enough_tps_to_send(void) {
	if (((m_tp_q.size() * 8) + 80) >= (size_t)m_format.kbch)
		return true;
	else
		return false;
}
//
// We have enough bytes to transmit
//
void DVB2::dvb2_unpack_and_prepare_for_interleaving_and_ldpc(void) {
	// Add the header
	int index = dvb2_add_bbheader_bytes();
	// Calculate the number of payload bytes
	int nb = (m_format.kbch/8) - index;
	// Add those bytes starting after the header
	for (int i = 0; i < nb; i++) {
		m_frame_bytes[index++] = m_tp_q.front();
		m_tp_q.pop();
	}
	// Apply the scrambler
	dvb2_bb_randomise_bytes();
	// Apply the BCH encoding
	dvb2_bch_byte_encode();
	// Now unpack into a bit array 
	dvb2_unpack_bytes(m_frame_bytes, m_frame, m_format.kldpc/8);
	// The number of overflowed bytes will tell us where the next
	// sync byte will start
	m_tp_overflow = m_tp_q.size();
}
