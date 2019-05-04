#include "DVB2.h"
#ifndef DVBS2_H
#define DVBS2_H

#ifndef M_PI
#define M_PI 3.14159f
#endif

#define loggerf printf

class DVBS2 : public DVB2{
private:
        const static unsigned long g[6];
        const static int ph_scram_tab[64];
        const static int ph_sync_seq[26];
        SComplex m_bpsk[2][2];
        SComplex m_qpsk[4];
        SComplex m_8psk[8];
        SComplex m_16apsk[16];
        SComplex m_32apsk[32];
        SComplex m_pl[FRAME_SIZE_NORMAL];
        SComplex m_pl_dummy[FRAME_SIZE_NORMAL];
        int m_cscram[FRAME_SIZE_NORMAL];
        int m_iframe[FRAME_SIZE_NORMAL];
        int m_payload_symbols;
        int m_dummy_frame_length;
	int m_configured;
	int m_last_modcod;
	int m_current_modcod;
        double m_efficiency;
	void(*m_tx)(SComplex *s, int len);
	void s2_b_64_7_code( unsigned char in, int *out );
        void s2_pl_header_encode( uint8_t modcod, uint8_t type, int *out);
        void s2_modulator_configuration(void);
	void s2_interleave( void );
        void s2_pl_header_create(void);
        int  s2_pl_data_pack( void );
        void s2_pl_scramble_symbols( SComplex *fs, int len );
        void s2_pl_scramble_dummy_symbols( int len );
        void s2_pl_build_dummy( void );
        int s2_parity_chk( long a, long b);
        void s2_build_symbol_scrambler_table( void );
        void s2_calc_efficiency( void );
	double s2_fiddle_16apsk(double y);
	double s2_fiddle_32apsk(double ya, double yb);

public:
	void   s2_reset(void);
	void   s2_register_tx(void(*f)(SComplex *s, int len));
	int    s2_is_valid(int type, int mod, int coderate);		
	double s2_get_efficiency( void );
        void   s2_physical(void);
        int    s2_set_configure( DVB2FrameFormat *f );
        void   s2_get_configure( DVB2FrameFormat *f );
        SComplex *s2_pl_get_frame(void);
        SComplex *s2_pl_get_dummy( int &len );
        int     s2_add_ts_frame( uint8_t *ts );
	DVBS2();
};

#endif
