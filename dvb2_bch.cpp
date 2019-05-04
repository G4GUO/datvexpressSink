// polymult.cpp : Defines the entry point for the console application.
//
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
#include "DVBS2.h"

// BCH byte lookup table

//
// Display routines.
//
void display_poly( int *in, int len )
{
        loggerf("\n");
	for( int i = 0; i < len; i++ )
	{
		if(in[i] == 1 )
		{
			if( i == 0 )
                                loggerf("1");
			else
				if( i == 1 )
                                        loggerf("+x");
				else
                                        loggerf("+x^%d",i);
		}
	}
        loggerf("\n");
}
//
// length is in bits
//
void display_poly_pack( unsigned int *in, int len )
{

   //     loggerf("\n");
	for( int i = 0; i < len/32; i++ )
	{
               // loggerf("%.8X",in[i]);
	}
	switch((len%32)/8)
	{
	case 0:
		break;
	case 1:
               // loggerf("%.2X",in[(len/32)]>>24);
		break;
	case 2:
              //  loggerf("%.2X",in[(len/32)]>>24);
             //   loggerf("%.2X",in[(len/32)]>>16);
		break;
	case 3:
             //   loggerf("%.2X",in[(len/32)]>>24);
             //   loggerf("%.2X",in[(len/32)]>>16);
             //   loggerf("%.2X",in[(len/32)]>>8);
		break;
	}
      //  loggerf("\n");
}

//
// Polynomial calculation routines
//
// multiply polynomials
//
int DVB2::dvb2_poly_mult( const int *ina, int lena, const int *inb, int lenb, int *out )
{
    memset( out, 0, sizeof(int)*(lena+lenb));

    for( int i = 0; i < lena; i++ )
    {
        for( int j = 0; j < lenb; j++ )
        {
            if( ina[i]*inb[j] > 0 ) out[i+j]++;// count number of terms for this pwr of x
        }
    }
    int max=0;
    for( int i = 0; i < lena+lenb; i++ )
    {
        out[i] = out[i]&1;// If even ignore the term
        if(out[i]) max = i;
    }
    // return the size of array to house the result.
    return max+1;

}
//
// Pack the polynomial into a 32 bit array
//

void DVB2::dvb2_poly_32_pack( const int *pin, uint32_t* pout, int len )
{
    int lw = len/32;
    int ptr = 0;
    uint32_t temp;
    if( len % 32 ) lw++;

    for( int i = 0; i < lw; i++ )
    {
        temp    = 0x80000000;
        pout[i] = 0;
        for( int j = 0; j < 32; j++ )
        {
            if( pin[ptr++] ) pout[i] |= temp;
            temp >>= 1;
        }
    }
}

void DVB2::dvb2_poly_reverse( int *pin, int *pout, int len )
{
    int c;
    c = len-1;

    for( int i = 0; i < len; i++ )
    {
        pout[c--] = pin[i];
    }
}

////////////////////////////////////////////////////////////////////////
//
// Build the 64 bit based lookup tables
// This is only called at start up so speed is not important
//
////////////////////////////////////////////////////////////////////////

void  DVB2::dvb2_cb_sr_32_to_64(uint32_t *in, BCHLookup *out, int n)
{
	int idx = 0;
	for (int i = 0; i < n; i++) {
		out->bch_r[i] = in[idx++];
		out->bch_r[i] <<= 32;
		out->bch_r[i] |= in[idx++];
	}
}

//
// Shift a 128 bit register
//
void  inline reg_32_4_shift(uint32_t *sr)
{
	sr[0] = (sr[0] << 1) | (sr[1] >> 31);
	sr[1] = (sr[1] << 1) | (sr[2] >> 31);
	sr[2] = (sr[2] << 1) | (sr[3] >> 31);
	sr[3] = (sr[3] << 1);
}
//
// Shift 160 bits
//
void  inline reg_32_5_shift(uint32_t *sr)
{
	sr[0] = (sr[0] << 1) | (sr[1] >> 31);
	sr[1] = (sr[1] << 1) | (sr[2] >> 31);
	sr[2] = (sr[2] << 1) | (sr[3] >> 31);
	sr[3] = (sr[3] << 1) | (sr[4] >> 31);
	sr[4] = (sr[4] << 1);
}
//
// Shift 192 bits
//
void  inline reg_32_6_shift(uint32_t *sr)
{
	sr[0] = (sr[0] << 1) | (sr[1] >> 31);
	sr[1] = (sr[1] << 1) | (sr[2] >> 31);
	sr[2] = (sr[2] << 1) | (sr[3] >> 31);
	sr[3] = (sr[3] << 1) | (sr[4] >> 31);
	sr[4] = (sr[4] << 1) | (sr[5] >> 31);
	sr[5] = (sr[5] << 1);
}

void  DVB2::dvb2_bch_n_8_parity_check_lookup_table(void)
{
	uint32_t b;
	uint32_t shift[4];

	for (uint32_t n = 0; n <= 255; n++) {
		//Zero the shift register
		memset(shift, 0, sizeof(uint32_t) * 4);
		shift[0] = (n << 24);
		for (int i = 0; i < 8; i++)
		{
			b = (shift[0] & 0x80000000);
			reg_32_4_shift(shift);
			if (b)
			{
				shift[0] ^= m_32_poly_n_8[0];
				shift[1] ^= m_32_poly_n_8[1];
				shift[2] ^= m_32_poly_n_8[2];
				shift[3] ^= m_32_poly_n_8[3];
			}
		}
		dvb2_cb_sr_32_to_64(shift, &m_bch_n8_lookup[n], 2);
	}
}

void  DVB2::dvb2_bch_n_10_parity_check_lookup_table(void)
{
	uint32_t b;
	uint32_t shift[5];

	for (uint32_t n = 0; n <= 255; n++) {
		//Zero the shift register
		memset(shift, 0, sizeof(uint32_t) * 5);
		shift[0] = (n << 24);
		for (int i = 0; i < 8; i++)
		{
			b = (shift[0] & 0x80000000);
			reg_32_5_shift(shift);
			if (b)
			{
				shift[0] ^= m_32_poly_n_10[0];
				shift[1] ^= m_32_poly_n_10[1];
				shift[2] ^= m_32_poly_n_10[2];
				shift[3] ^= m_32_poly_n_10[3];
				shift[4] ^= m_32_poly_n_10[4];
			}
		}
		dvb2_cb_sr_32_to_64(shift, &m_bch_n10_lookup[n], 3);
		m_bch_n10_lookup[n].bch_r[2] &= 0xFFFFFFFF00000000;
	}
}

void  DVB2::dvb2_bch_n_12_parity_check_lookup_table(void)
{
	uint32_t b;
	uint32_t shift[6];
	for (uint32_t n = 0; n <= 255; n++) {
		//Zero the shift register
		memset(shift, 0, sizeof(uint32_t) * 6);
		shift[0] = (n << 24);
		for (int i = 0; i < 8; i++)
		{
			b = (shift[0] & 0x80000000);
			reg_32_6_shift(shift);
			if (b)
			{
				shift[0] ^= m_32_poly_n_12[0];
				shift[1] ^= m_32_poly_n_12[1];
				shift[2] ^= m_32_poly_n_12[2];
				shift[3] ^= m_32_poly_n_12[3];
				shift[4] ^= m_32_poly_n_12[4];
				shift[5] ^= m_32_poly_n_12[5];
			}
		}
		dvb2_cb_sr_32_to_64(shift, &m_bch_n12_lookup[n], 3);
	}
}

void  DVB2::dvb2_bch_s_12_parity_check_lookup_table(void)
{
	uint32_t b;
	uint32_t shift[6];

	for (uint32_t n = 0; n <= 255; n++) {
		//Zero the shift register
		memset(shift, 0, sizeof(uint32_t) * 6);
		shift[0] = (n << 24);
		for (int i = 0; i < 8; i++)
		{
			b = (shift[0] & 0x80000000);
			reg_32_6_shift(shift);
			if (b)
			{
				shift[0] ^= m_32_poly_s_12[0];
				shift[1] ^= m_32_poly_s_12[1];
				shift[2] ^= m_32_poly_s_12[2];
				shift[3] ^= m_32_poly_s_12[3];
				shift[4] ^= m_32_poly_s_12[4];
				shift[5] ^= m_32_poly_s_12[5];
			}
		}
		dvb2_cb_sr_32_to_64(shift, &m_bch_s12_lookup[n], 3);
		m_bch_s12_lookup[n].bch_r[2] &= 0xFFFFFFFFFF000000;
	}
}

void  DVB2::dvb2_bch_m_12_parity_check_lookup_table(void)
{
	uint32_t b;
	uint32_t shift[6];

	for (int n = 0; n <= 255; n++) {
		//Zero the shift register
		memset(shift, 0, sizeof(uint32_t) * 6);
		shift[0] = (n << 24);
		for (int i = 0; i < 8; i++)
		{
			b = (shift[0] & 0x80000000);
			reg_32_6_shift(shift);
			if (b)
			{
				shift[0] ^= m_32_poly_m_12[0];
				shift[1] ^= m_32_poly_m_12[1];
				shift[2] ^= m_32_poly_m_12[2];
				shift[3] ^= m_32_poly_m_12[3];
				shift[4] ^= m_32_poly_m_12[4];
				shift[5] ^= m_32_poly_m_12[5];
			}
		}
		dvb2_cb_sr_32_to_64(shift, &m_bch_m12_lookup[n], 3);
		m_bch_n10_lookup[n].bch_r[2] &= 0xFFFFFFFFFFFFF000;
	}
}
//////////////////////////////////////////////////////////////////////
//
// The actual parity check
//
// This encodes in place, len is the number of un coded bytes
// it returns the new size of the array
//
//////////////////////////////////////////////////////////////////////

//
// 64 bit shifts
//
void  inline reg_64_2_shift(uint64_t *sr)
{
	sr[0] = (sr[0] << 8) | (sr[1] >> 56);
	sr[1] = (sr[1] << 8);
}
void  inline reg_64_3_shift(uint64_t *sr)
{
	sr[0] = (sr[0] << 8) | (sr[1] >> 56);
	sr[1] = (sr[1] << 8) | (sr[2] >> 56);
	sr[2] = (sr[2] << 8);
}
// 128 parity bits 
int  DVB2::dvb2_bch_byte_n_8_parity_encode(uint8_t *inout, int len) {
	uint64_t shift[2];
	uint8_t b;
	int i;
	//Zero the shift register 128 bits

	shift[0] = 0;
	shift[1] = 0;

	for ( i = 0; i < len; i++) {
		b = ((uint8_t)(shift[0] >> 56)) ^ inout[i];
		reg_64_2_shift(shift);
		shift[0] ^= m_bch_n8_lookup[b].bch_r[0];
		shift[1] ^= m_bch_n8_lookup[b].bch_r[1];
	}

	for (i = 0; i < 16; i++) {
		inout[len++] = ((uint8_t)(shift[0] >> 56));
		reg_64_2_shift(shift);
	}

	return len;
}
// 160 parity bits
int  DVB2::dvb2_bch_byte_n_10_parity_encode(uint8_t *inout, int len) {
	uint64_t shift[3];
	uint8_t b;
	int i;
	//Zero the shift register 160 bits
	memset(shift, 0, sizeof(uint64_t) * 3);

	for (i = 0; i < len; i++) {
		b = ((uint8_t)(shift[0] >> 56)) ^ inout[i];
		reg_64_3_shift(shift);
		shift[0] ^= m_bch_n10_lookup[b].bch_r[0];
		shift[1] ^= m_bch_n10_lookup[b].bch_r[1];
		shift[2] ^= m_bch_n10_lookup[b].bch_r[2];
	}

	for (i = 0; i < 20; i++) {
		inout[len++] = ((uint8_t)(shift[0] >> 56));
		reg_64_3_shift(shift);
	}

	return len;
}
// 192 parity bits
int  DVB2::dvb2_bch_byte_n_12_parity_encode(uint8_t *inout, int len) {
	uint64_t shift[3];
	uint8_t b;
	int i;
	//Zero the shift register 192 bits
	memset(shift, 0, sizeof(uint64_t) * 3);

	for ( i = 0; i < len; i++) {
		b = ((uint8_t)(shift[0] >> 56)) ^ inout[i];
		reg_64_3_shift(shift);
		shift[0] ^= m_bch_n12_lookup[b].bch_r[0];
		shift[1] ^= m_bch_n12_lookup[b].bch_r[1];
		shift[2] ^= m_bch_n12_lookup[b].bch_r[2];
	}

	for (i = 0; i < 24; i++) {
		inout[len++] = ((uint8_t)(shift[0] >> 56));
		reg_64_3_shift(shift);
	}

	return len;
}
// 168 parity bits
int  DVB2::dvb2_bch_byte_s_12_parity_encode(uint8_t *inout, int len) {
	uint64_t shift[3];
	uint8_t b;
	int i;
	//Zero the shift register 168 bits
	memset(shift, 0, sizeof(uint64_t) * 3);

	for ( i = 0; i < len; i++) {
		b = ((uint8_t)(shift[0] >> 56)) ^ inout[i];
		reg_64_3_shift(shift);
		shift[0] ^= m_bch_s12_lookup[b].bch_r[0];
		shift[1] ^= m_bch_s12_lookup[b].bch_r[1];
		shift[2] ^= m_bch_s12_lookup[b].bch_r[2];
	}

	for (i = 0; i < 21; i++) {
		inout[len++] = ((uint8_t)(shift[0] >> 56));
		reg_64_3_shift(shift);
	}

	return len;
}
// 180 parity bits
int  DVB2::dvb2_bch_byte_m_12_parity_encode(uint8_t *inout, int len) {
	uint64_t shift[3];
	uint8_t b;
	int i;
	//Zero the shift register 180 bits
	memset(shift, 0, sizeof(uint64_t) * 3);

	for ( i = 0; i < len; i++) {
		b = ((uint8_t)(shift[0] >> 56)) ^ inout[i];
		reg_64_3_shift(shift);
		shift[0] ^= m_bch_m12_lookup[b].bch_r[0];
		shift[1] ^= m_bch_m12_lookup[b].bch_r[1];
		shift[2] ^= m_bch_m12_lookup[b].bch_r[2];
	}

	for (i = 0; i < 23; i++) {
		inout[len++] = ((uint8_t)(shift[0] >> 56));
		reg_64_3_shift(shift);
	}

	return len;//Actually 22.5
}

int DVB2::dvb2_bch_byte_encode(void)
{
	int len = m_format.kbch/8;

	switch (m_format.bch_code)
	{
	case BCH_CODE_N8:
		len = dvb2_bch_byte_n_8_parity_encode( m_frame_bytes, len);
		break;
	case BCH_CODE_N10:
		len = dvb2_bch_byte_n_10_parity_encode(m_frame_bytes, len);
		break;
	case BCH_CODE_N12:
		len = dvb2_bch_byte_n_12_parity_encode(m_frame_bytes, len);
		break;
	case BCH_CODE_S12:
		len = dvb2_bch_byte_s_12_parity_encode(m_frame_bytes, len);
		break;
	default:
		len = dvb2_bch_byte_n_12_parity_encode(m_frame_bytes, len);
		break;
	}
	return len;
}

//
//
//
void DVB2::dvb2_bch_build_tables( void )
{
    // Normal polynomials
    const int polyn01[]={1,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,1};
    const int polyn02[]={1,1,0,0,1,1,1,0,1,0,0,0,0,0,0,0,1};
    const int polyn03[]={1,0,1,1,1,1,0,1,1,1,1,1,0,0,0,0,1};
    const int polyn04[]={1,0,1,0,1,0,1,0,0,1,0,1,1,0,1,0,1};
    const int polyn05[]={1,1,1,1,0,1,0,0,1,1,1,1,1,0,0,0,1};
    const int polyn06[]={1,0,1,0,1,1,0,1,1,1,1,0,1,1,1,1,1};
    const int polyn07[]={1,0,1,0,0,1,1,0,1,1,1,1,0,1,0,1,1};
    const int polyn08[]={1,1,1,0,0,1,1,0,1,1,0,0,1,1,1,0,1};
    const int polyn09[]={1,0,0,0,0,1,0,1,0,1,1,1,0,0,0,0,1};
    const int polyn10[]={1,1,1,0,0,1,0,1,1,0,1,0,1,1,1,0,1};
    const int polyn11[]={1,0,1,1,0,1,0,0,0,1,0,1,1,1,0,0,1};
    const int polyn12[]={1,1,0,0,0,1,1,1,0,1,0,1,1,0,0,0,1};

    // Short polynomials
    const int polys01[]={1,1,0,1,0,1,0,0,0,0,0,0,0,0,1};
    const int polys02[]={1,0,0,0,0,0,1,0,1,0,0,1,0,0,1};
    const int polys03[]={1,1,1,0,0,0,1,0,0,1,1,0,0,0,1};
    const int polys04[]={1,0,0,0,1,0,0,1,1,0,1,0,1,0,1};
    const int polys05[]={1,0,1,0,1,0,1,0,1,1,0,1,0,1,1};
    const int polys06[]={1,0,0,1,0,0,0,1,1,1,0,0,0,1,1};
    const int polys07[]={1,0,1,0,0,1,1,1,0,0,1,1,0,1,1};
    const int polys08[]={1,0,0,0,0,1,0,0,1,1,1,1,0,0,1};
    const int polys09[]={1,1,1,1,0,0,0,0,0,1,1,0,0,0,1};
    const int polys10[]={1,0,0,1,0,0,1,0,0,1,0,1,1,0,1};
    const int polys11[]={1,0,0,0,1,0,0,0,0,0,0,1,1,0,1};
    const int polys12[]={1,1,1,1,0,1,1,1,1,0,1,0,0,1,1};

    int len;
    int polyout[2][200];

	memset(polyout[0], 0, sizeof(int) * 200);
	memset(polyout[1], 0, sizeof(int) * 200);

    len = dvb2_poly_mult( polyn01, 17, polyn02,    17,  polyout[0] );
    len = dvb2_poly_mult( polyn03, 17, polyout[0], len, polyout[1] );
    len = dvb2_poly_mult( polyn04, 17, polyout[1], len, polyout[0] );
    len = dvb2_poly_mult( polyn05, 17, polyout[0], len, polyout[1] );
    len = dvb2_poly_mult( polyn06, 17, polyout[1], len, polyout[0] );
    len = dvb2_poly_mult( polyn07, 17, polyout[0], len, polyout[1] );
    len = dvb2_poly_mult( polyn08, 17, polyout[1], len, polyout[0] );
	dvb2_poly_reverse(polyout[0], polyout[1], 128);
	dvb2_poly_32_pack( polyout[1], m_32_poly_n_8, 128 );

    len = dvb2_poly_mult( polyn09, 17, polyout[0], len, polyout[1] );
    len = dvb2_poly_mult( polyn10, 17, polyout[1], len, polyout[0] );
	dvb2_poly_reverse(polyout[0], polyout[1], 160);
	dvb2_poly_32_pack( polyout[1], m_32_poly_n_10, 160 );

    len = dvb2_poly_mult( polyn11, 17, polyout[0], len, polyout[1] );
    len = dvb2_poly_mult( polyn12, 17, polyout[1], len, polyout[0] );
	dvb2_poly_reverse(polyout[0], polyout[1], 192);
	dvb2_poly_32_pack( polyout[1], m_32_poly_n_12, 192 );

	memset(polyout[0], 0, sizeof(int) * 200);
	memset(polyout[1], 0, sizeof(int) * 200);

    len = dvb2_poly_mult( polys01, 15, polys02,    15,  polyout[0] );
    len = dvb2_poly_mult( polys03, 15, polyout[0], len, polyout[1] );
    len = dvb2_poly_mult( polys04, 15, polyout[1], len, polyout[0] );
    len = dvb2_poly_mult( polys05, 15, polyout[0], len, polyout[1] );
    len = dvb2_poly_mult( polys06, 15, polyout[1], len, polyout[0] );
    len = dvb2_poly_mult( polys07, 15, polyout[0], len, polyout[1] );
    len = dvb2_poly_mult( polys08, 15, polyout[1], len, polyout[0] );
    len = dvb2_poly_mult( polys09, 15, polyout[0], len, polyout[1] );
    len = dvb2_poly_mult( polys10, 15, polyout[1], len, polyout[0] );
    len = dvb2_poly_mult( polys11, 15, polyout[0], len, polyout[1] );
    len = dvb2_poly_mult( polys12, 15, polyout[1], len, polyout[0] );
	dvb2_poly_reverse(polyout[0], polyout[1], 168);
	dvb2_poly_32_pack( polyout[1], m_32_poly_s_12, 168 );

	// Build the lookup tables for the fast encoder
	dvb2_bch_n_8_parity_check_lookup_table();
	dvb2_bch_n_10_parity_check_lookup_table();
	dvb2_bch_n_12_parity_check_lookup_table();
	dvb2_bch_s_12_parity_check_lookup_table();
	//bch_m_12_parity_check_lookup_table();

}
