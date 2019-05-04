#include "DVB2.h"

//
// Pack into 8 bit array
//
void DVB2::dvb2_init_bb_randomiser_bytes(void)
{
	uint16_t sr = 0x4A80;
	for (int i = 0; i < FRAME_SIZE_NORMAL/8; i++)
	{
		m_bb_randomise[i] = 0;
		for (int j = 0; j < 8; j++) {
			int b = ((sr) ^ (sr >> 1)) & 1;
			sr >>= 1;
			if (b) sr |= 0x4000;
			m_bb_randomise[i] = (m_bb_randomise[i]<<1) | b;
		}
	}
}
//
// Randomise the data bytes
//
void DVB2::dvb2_bb_randomise_bytes(void)
{
	for (int i = 0; i < m_format.kbch/8; i++)
	{
		m_frame_bytes[i] ^= m_bb_randomise[i];
	}
}
