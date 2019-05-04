#include "memory.h"
#include "DVB2.h"



//
// This configures the system and calculates
// any required intermediate values
//
int DVB2::dvb2_set_configure( DVB2FrameFormat *f )
{
    int bch_bits = 0;
    int error = 0;

    if( f->broadcasting )
    {
        // Set standard parametrs for broadcasting
        f->bb_header.ts_gs   = TS_GS_TRANSPORT;
        f->bb_header.sis_mis = SIS_MIS_SINGLE;
        f->bb_header.ccm_acm = CCM;
        f->bb_header.issyi   = 0;
        f->bb_header.npd     = 0;
        f->bb_header.upl     = 188*8;
        f->bb_header.sync    = 0x47;
    }
    f->bb_header.ro = f->roll_off;
    // Fill in the mode specific values and bit lengths
    if( f->frame_type == FRAME_NORMAL )
    {
        f->nldpc = 64800;
        bch_bits = 192;
        f->bch_code = BCH_CODE_N12;
        // Apply code rate
        switch(f->code_rate )
        {
            case CR_1_4:
                f->q_val = 135;
                f->kbch  = 16008;
                f->bch_code = BCH_CODE_N12;
                break;
            case CR_1_3:
                f->q_val = 120;
                f->kbch  = 21408;
                f->bch_code = BCH_CODE_N12;
                break;
            case CR_2_5:
                f->q_val = 108;
                f->kbch  = 25728;
                f->bch_code = BCH_CODE_N12;
                break;
            case CR_1_2:
                f->q_val = 90;
                f->kbch  = 32208;
                f->bch_code = BCH_CODE_N12;
                break;
            case CR_3_5:
                f->q_val = 72;
                f->kbch  = 38688;
                f->bch_code = BCH_CODE_N12;
                break;
            case CR_2_3:
                bch_bits = 160;
                f->q_val = 60;
                f->kbch  = 43040;
                f->bch_code = BCH_CODE_N10;
                break;
            case CR_3_4:
                f->q_val = 45;
                f->kbch  = 48408;
                f->bch_code = BCH_CODE_N12;
                break;
            case CR_4_5:
                f->q_val = 36;
                f->kbch  = 51648;
                f->bch_code = BCH_CODE_N12;
                break;
            case CR_5_6:
                bch_bits = 160;
                f->q_val = 30;
                f->kbch  = 53840;
                f->bch_code = BCH_CODE_N10;
                break;
            case CR_8_9:
                bch_bits = 128;
                f->q_val = 20;
                f->kbch  = 57472;
                f->bch_code = BCH_CODE_N8;
                break;
            case CR_9_10:
                bch_bits = 128;
                f->q_val = 18;
                f->kbch  = 58192;
                f->bch_code = BCH_CODE_N8;
                break;
            default:
                error = -1;
                break;
        }
    }

    if( f->frame_type == FRAME_SHORT )
    {
        f->nldpc = 16200;
        bch_bits = 168;
        f->bch_code = BCH_CODE_S12;
        // Apply code rate
        switch(f->code_rate )
        {
            case CR_1_4:
                f->q_val = 36;
                f->kbch  = 3072;
                break;
            case CR_1_3:
                f->q_val = 30;
                f->kbch  = 5232;
                break;
            case CR_2_5:
                f->q_val = 27;
                f->kbch  = 6312;
                break;
            case CR_1_2:
                f->q_val = 25;
                f->kbch  = 7032;
                break;
            case CR_3_5:
                f->q_val = 18;
                f->kbch  = 9552;
                break;
            case CR_2_3:
                f->q_val = 15;
                f->kbch  = 10632;
                break;
            case CR_3_4:
                f->q_val = 12;
                f->kbch  = 11712;
                break;
            case CR_4_5:
                f->q_val = 10;
                f->kbch  = 12432;
                break;
            case CR_5_6:
                f->q_val = 8;
                f->kbch  = 13152;
                break;
            case CR_8_9:
                f->q_val = 5;
                f->kbch  = 14232;
                break;
            default:
                error = -1;
                break;
        }
    }
    if( error == 0 )
    {
        // Length of the user packets
        f->bb_header.upl  = 188*8;
        // Payload length
        f->bb_header.dfl = f->kbch - 80;
        // Transport packet sync
        f->bb_header.sync = 0x47;
        // Start of LDPC bits
        f->kldpc = f->kbch + bch_bits;
        // Number of padding bits required (not used)
        f->padding_bits = 0;
        // Number of useable data bits (not used)
		f->useable_data_bits = f->kbch - 80;
        // Save the configuration, will be updated on next frame
        m_format = *f;
        // reset various pointers
        m_dnp   = 0;// No deleted null packets
    }
    return error;
}
void DVB2::dvb2_get_configure( DVB2FrameFormat *f )
{
    *f = m_format;
}

// Points to first byte in transport packet

int DVB2::dvb2_add_ts_frame_base( uint8_t *ts )
{
	// Queue the transport packet for transmission
	dvb2_add_crc_to_tp_and_queue(ts);
	// Do we have enough packets to send
	if (dvb2_enough_tps_to_send() == true) {
		dvb2_unpack_and_prepare_for_interleaving_and_ldpc();
		// LDPC encode the BB frame and BCHFEC bits
		dvb2_ldpc_encode();
		// Signal to the modulation specific class we have something to send
		return 1;
	}
    return 0;
}
//
// Dump NULL packets appends a counter to the end of each UP
// it is not implemented at the moment.
//
int DVB2::dvb2_next_ts_frame_base( uint8_t *ts )
{
    int res = 0;
    // See if we need to dump null packets
    if( m_format.null_deletion == 1 )
    {
        if(((ts[0]&0x1F) == 0x1F)&&(ts[1] == 0xFF ))
        {
            // Null packet detected
            if( m_dnp < 0xFF )
            {
                m_dnp++;// Increment the number of null packets
                return 0;
            }
        }
    }
    // Need to send a new transport packet 
    res = dvb2_add_ts_frame_base( ts );
    if( res ) m_dnp = 0;// Clear the DNP counter
    // return whether it is time to transmit a new frame
    return res;
}
void DVB2::dvb2_reset(void) {
	m_dnp = 0;// No delted null packets
	while(!m_tp_q.empty()) m_tp_q.pop();
}
DVB2::DVB2(void)
{
    // Clear the transport queue
    m_tp_q.empty();
	dvb2_init_bb_randomiser_bytes();
	dvb2_bch_build_tables();
	dvb2_build_crc8_table();
    m_dnp   = 0;// No delted null packets
}
DVB2::~DVB2(void)
{
}
