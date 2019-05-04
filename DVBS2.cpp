#include "memory.h"
#include "DVBS2.h"

int DVBS2::s2_is_valid( int type, int mod, int coderate )
{
	if (type == FRAME_NORMAL) {
		if (mod == M_QPSK)
		{
			if (coderate == CR_1_4)  return 0;
			if (coderate == CR_1_3)  return 0;
			if (coderate == CR_2_5)  return 0;
			if (coderate == CR_1_2)  return 0;
			if (coderate == CR_3_5)  return 0;
			if (coderate == CR_2_3)  return 0;
			if (coderate == CR_3_4)  return 0;
			if (coderate == CR_4_5)  return 0;
			if (coderate == CR_5_6)  return 0;
			if (coderate == CR_8_9)  return 0;
			if (coderate == CR_9_10) return 0;
		}
		if (mod == M_8PSK)
		{
			if (coderate == CR_3_5)  return 0;
			if (coderate == CR_2_3)  return 0;
			if (coderate == CR_3_4)  return 0;
			if (coderate == CR_5_6)  return 0;
			if (coderate == CR_8_9)  return 0;
			if (coderate == CR_9_10) return 0;
		}
		if (mod == M_16APSK)
		{
			if (coderate == CR_2_3)  return 0;
			if (coderate == CR_3_4)  return 0;
			if (coderate == CR_4_5)  return 0;
			if (coderate == CR_5_6)  return 0;
			if (coderate == CR_8_9)  return 0;
			if (coderate == CR_9_10) return 0;
		}
		if (mod == M_32APSK)
		{
			if (coderate == CR_3_4)  return 0;
			if (coderate == CR_4_5)  return 0;
			if (coderate == CR_5_6)  return 0;
			if (coderate == CR_8_9)  return 0;
			if (coderate == CR_9_10) return 0;
		}
	}
	if (type == FRAME_SHORT) {
		if (mod == M_QPSK)
		{
			if (coderate == CR_1_4)  return 0;
			if (coderate == CR_1_3)  return 0;
			if (coderate == CR_2_5)  return 0;
			if (coderate == CR_1_2)  return 0;
			if (coderate == CR_3_5)  return 0;
			if (coderate == CR_2_3)  return 0;
			if (coderate == CR_3_4)  return 0;
			if (coderate == CR_4_5)  return 0;
			if (coderate == CR_5_6)  return 0;
			if (coderate == CR_8_9)  return 0;
		}
		if (mod == M_8PSK)
		{
			if (coderate == CR_3_5)  return 0;
			if (coderate == CR_2_3)  return 0;
			if (coderate == CR_3_4)  return 0;
			if (coderate == CR_5_6)  return 0;
			if (coderate == CR_8_9)  return 0;
		}
		if (mod == M_16APSK)
		{
			if (coderate == CR_2_3)  return 0;
			if (coderate == CR_3_4)  return 0;
			if (coderate == CR_4_5)  return 0;
			if (coderate == CR_5_6)  return 0;
			if (coderate == CR_8_9)  return 0;
		}
		if (mod == M_32APSK)
		{
			if (coderate == CR_3_4)  return 0;
			if (coderate == CR_4_5)  return 0;
			if (coderate == CR_5_6)  return 0;
			if (coderate == CR_8_9)  return 0;
		}
	}

    return -1;
}
//
// index 0 and 1 will only be different when being reconfigured.
// Use index 1 as this will be applied in the following transmit
// frames
//
void DVBS2::s2_calc_efficiency( void )
{
    double p,m,a,s,po,tfl;
    // Calculate the number of symbols in the payload
	p = 0;a = 0; m = 0; po = 0;
    if( m_format.frame_type == FRAME_NORMAL )  p = (double)FRAME_SIZE_NORMAL;
    if( m_format.frame_type == FRAME_SHORT  )  p = (double)FRAME_SIZE_SHORT;
    if( m_format.constellation == M_QPSK )     m = 2.0;
    if( m_format.constellation == M_8PSK )     m = 3.0;
    if( m_format.constellation == M_16APSK )   m = 4.0;
    if( m_format.constellation == M_32APSK )   m = 5.0;
    s = p/m;//Number of data carrying symbols per frame
    // PL header overhead
    if( m_format.pilots ){
        po = (s/(90*16))-1;// 1 pilot every 16 blocks (of 90 symbols)
        po = po*36;        // No pilot at the end
    }
	tfl = 90 + s + po;
	m_efficiency = ((double)m_format.useable_data_bits) / tfl;
}
//
// Multiply the efficiency value by the symbol rate
// to get the useable bitrate
//
double DVBS2::s2_get_efficiency( void )
{
    return m_efficiency;
}

int DVBS2::s2_set_configure( DVB2FrameFormat *f )
{
    if( s2_is_valid( f->frame_type, f->constellation, f->code_rate ) == 0 )
    {
        if(dvb2_set_configure( f ) == 0 )
        {
			s2_calc_efficiency();
			s2_pl_header_create();
			if (m_current_modcod != m_last_modcod) {
				s2_modulator_configuration();
				// Update the LDPC
				dvb2_ldpc_lookup_generate();
			}
			m_last_modcod = m_current_modcod;
			m_configured = 1;
            return 0;
        }
    }
    return -1;
}
void DVBS2::s2_get_configure( DVB2FrameFormat *f )
{
	dvb2_get_configure( f );
}
int DVBS2::s2_add_ts_frame( uint8_t *ts )
{
    int res = 0;
	if (m_configured == 0) return 0;
    // Call base class
    if(dvb2_next_ts_frame_base( ts ) )
    {
        // Interleave and pack
        s2_interleave();
        // create the header
        s2_pl_header_create();
        // Add the data
        res = s2_pl_data_pack();
		if (m_tx != NULL) {
			m_tx(m_pl, res);
			res = 0;
		}
    }
    return res;
}
//
// Pass pointer to the transmit sample function
// if not called then no function is called
//
void DVBS2::s2_register_tx(void(*f)(SComplex *s, int len)) {
	m_tx = f;
}

void DVBS2::s2_physical( void )
{

}
void DVBS2::s2_reset(void) {
//	m_configured = 0;
	m_last_modcod = -1;//Set to an illegal value
	dvb2_reset();
}
DVBS2::DVBS2()
{
	m_configured = 0;
	m_last_modcod = -1;//Set to an illegal value
	m_tx = NULL;
    s2_modulator_configuration();
    s2_build_symbol_scrambler_table();
    s2_pl_build_dummy();
}

