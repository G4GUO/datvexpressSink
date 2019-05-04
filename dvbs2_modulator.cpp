#include "math.h"
#include "DVBS2.h"

#define CP 0x7FFF

double DVBS2::s2_fiddle_16apsk(double y) {
	double r0,r1,r2;
	r2 = 1.0;
	r1 = r2 / y;

	if (m_format.coincedent_constellation == false)
		r0 = sqrt(((r1*r1) + 3.0*(r2*r2)) / 4.0);
	else
		r0 = 1.0;

	return r0;
}
double DVBS2::s2_fiddle_32apsk(double ya, double yb) {
	double r0, r1, r2, r3;
	r3 = 1.0;
	r1 = r3 / yb;
	r2 = r1 * ya;

	if (m_format.coincedent_constellation == false)
		r0 = sqrt(((r1*r1) + 3.0*(r2*r2) + 4.0*(r3*r3)) / 8.0);
	else
		r0 = 1.0;

	return r0;
}
void DVBS2::s2_modulator_configuration(void)
{
        double r0,r1,r2,r3;
		
		//
		// R0 values are approximate values, it is calculated using the energy 
		// calculation in the DVB-S2 spec. Basically I have normalised the 
		// magnitude of the symbol energy, this causes a reduction in the 
		// preamble / pilot tone magnitudes in any modes with QAM modulation.
		//

		if (m_format.constellation == M_QPSK) {
			r0 = 1;
			r1 = 1;
			// QPSK
			m_qpsk[0].re = (short)((r1*cos(M_PI / 4.0))*CP);
			m_qpsk[0].im = (short)((r1*sin(M_PI / 4.0))*CP);
			m_qpsk[1].re = (short)((r1*cos(7 * M_PI / 4.0))*CP);
			m_qpsk[1].im = (short)((r1*sin(7 * M_PI / 4.0))*CP);
			m_qpsk[2].re = (short)((r1*cos(3 * M_PI / 4.0))*CP);
			m_qpsk[2].im = (short)((r1*sin(3 * M_PI / 4.0))*CP);
			m_qpsk[3].re = (short)((r1*cos(5 * M_PI / 4.0))*CP);
			m_qpsk[3].im = (short)((r1*sin(5 * M_PI / 4.0))*CP);
		}

		if (m_format.constellation == M_8PSK) {
			r0 = 1;
			r1 = 1;
			// 8PSK
			m_8psk[0].re = (short)((r1*cos(M_PI / 4.0))*CP);
			m_8psk[0].im = (short)((r1*sin(M_PI / 4.0))*CP);
			m_8psk[1].re = (short)((r1*cos(0.0))*CP);
			m_8psk[1].im = (short)((r1*sin(0.0))*CP);
			m_8psk[2].re = (short)((r1*cos(4 * M_PI / 4.0))*CP);
			m_8psk[2].im = (short)((r1*sin(4 * M_PI / 4.0))*CP);
			m_8psk[3].re = (short)((r1*cos(5 * M_PI / 4.0))*CP);
			m_8psk[3].im = (short)((r1*sin(5 * M_PI / 4.0))*CP);
			m_8psk[4].re = (short)((r1*cos(2 * M_PI / 4.0))*CP);
			m_8psk[4].im = (short)((r1*sin(2 * M_PI / 4.0))*CP);
			m_8psk[5].re = (short)((r1*cos(7 * M_PI / 4.0))*CP);
			m_8psk[5].im = (short)((r1*sin(7 * M_PI / 4.0))*CP);
			m_8psk[6].re = (short)((r1*cos(3 * M_PI / 4.0))*CP);
			m_8psk[6].im = (short)((r1*sin(3 * M_PI / 4.0))*CP);
			m_8psk[7].re = (short)((r1*cos(6 * M_PI / 4.0))*CP);
			m_8psk[7].im = (short)((r1*sin(6 * M_PI / 4.0))*CP);
		}

		if (m_format.constellation == M_16APSK) {
			// 16 APSK
			r2 = 1.0;
			switch (m_format.code_rate)
			{
			case CR_2_3:
				r0 = s2_fiddle_16apsk(3.15);
				r1 = r2 / 3.15;
				break;
			case CR_3_4:
				r0 = s2_fiddle_16apsk(2.85);
				r1 = r2 / 2.85;
				break;
			case CR_4_5:
				r0 = s2_fiddle_16apsk(2.75);
				r1 = r2 / 2.75;
				break;
			case CR_5_6:
				r0 = s2_fiddle_16apsk(2.70);
				r1 = r2 / 2.70;
				break;
			case CR_8_9:
				r0 = s2_fiddle_16apsk(2.60);
				r1 = r2 / 2.60;
				break;
			case CR_9_10:
				r0 = s2_fiddle_16apsk(2.57);
				r1 = r2 / 2.57;
				break;
			default:
				// Illegal
				r1 = 0;
				r0 = 0;
				break;
			}

			m_16apsk[0].re = (short)((r2*cos(M_PI / 4.0))*CP);
			m_16apsk[0].im = (short)((r2*sin(M_PI / 4.0))*CP);
			m_16apsk[1].re = (short)((r2*cos(-M_PI / 4.0))*CP);
			m_16apsk[1].im = (short)((r2*sin(-M_PI / 4.0))*CP);
			m_16apsk[2].re = (short)((r2*cos(3 * M_PI / 4.0))*CP);
			m_16apsk[2].im = (short)((r2*sin(3 * M_PI / 4.0))*CP);
			m_16apsk[3].re = (short)((r2*cos(-3 * M_PI / 4.0))*CP);
			m_16apsk[3].im = (short)((r2*sin(-3 * M_PI / 4.0))*CP);
			m_16apsk[4].re = (short)((r2*cos(M_PI / 12.0))*CP);
			m_16apsk[4].im = (short)((r2*sin(M_PI / 12.0))*CP);
			m_16apsk[5].re = (short)((r2*cos(-M_PI / 12.0))*CP);
			m_16apsk[5].im = (short)((r2*sin(-M_PI / 12.0))*CP);
			m_16apsk[6].re = (short)((r2*cos(11 * M_PI / 12.0))*CP);
			m_16apsk[6].im = (short)((r2*sin(11 * M_PI / 12.0))*CP);
			m_16apsk[7].re = (short)((r2*cos(-11 * M_PI / 12.0))*CP);
			m_16apsk[7].im = (short)((r2*sin(-11 * M_PI / 12.0))*CP);
			m_16apsk[8].re = (short)((r2*cos(5 * M_PI / 12.0))*CP);
			m_16apsk[8].im = (short)((r2*sin(5 * M_PI / 12.0))*CP);
			m_16apsk[9].re = (short)((r2*cos(-5 * M_PI / 12.0))*CP);
			m_16apsk[9].im = (short)((r2*sin(-5 * M_PI / 12.0))*CP);
			m_16apsk[10].re = (short)((r2*cos(7 * M_PI / 12.0))*CP);
			m_16apsk[10].im = (short)((r2*sin(7 * M_PI / 12.0))*CP);
			m_16apsk[11].re = (short)((r2*cos(-7 * M_PI / 12.0))*CP);
			m_16apsk[11].im = (short)((r2*sin(-7 * M_PI / 12.0))*CP);
			m_16apsk[12].re = (short)((r1*cos(M_PI / 4.0))*CP);
			m_16apsk[12].im = (short)((r1*sin(M_PI / 4.0))*CP);
			m_16apsk[13].re = (short)((r1*cos(-M_PI / 4.0))*CP);
			m_16apsk[13].im = (short)((r1*sin(-M_PI / 4.0))*CP);
			m_16apsk[14].re = (short)((r1*cos(3 * M_PI / 4.0))*CP);
			m_16apsk[14].im = (short)((r1*sin(3 * M_PI / 4.0))*CP);
			m_16apsk[15].re = (short)((r1*cos(-3 * M_PI / 4.0))*CP);
			m_16apsk[15].im = (short)((r1*sin(-3 * M_PI / 4.0))*CP);
		}

		if (m_format.constellation == M_32APSK) {
			// 32 APSK
			r3 = 1.0;
			switch (m_format.code_rate)
			{
			case CR_3_4:
				r0 = s2_fiddle_32apsk(2.84, 5.27);
				r1 = r3 / 5.27;
				r2 = r1*2.84;
				break;
			case CR_4_5:
				r0 = s2_fiddle_32apsk(2.72, 4.87);
				r1 = r3 / 4.87;
				r2 = r1*2.72;
				break;
			case CR_5_6:
				r0 = s2_fiddle_32apsk(2.64, 4.64);
				r1 = r3 / 4.64;
				r2 = r1*2.64;
				break;
			case CR_8_9:
				r0 = s2_fiddle_32apsk(2.54, 4.33);
				r1 = r3 / 4.33;
				r2 = r1*2.54;
				break;
			case CR_9_10:
				r0 = s2_fiddle_32apsk(2.53, 4.30);
				r1 = r3 / 4.30;
				r2 = r1*2.53;
				break;
			default:
				// Illegal
				r0 = 0;
				r1 = 0;
				r2 = 0;
				break;
			}

			m_32apsk[0].re = (short)((r2*cos(M_PI / 4.0))*CP);
			m_32apsk[0].im = (short)((r2*sin(M_PI / 4.0))*CP);
			m_32apsk[1].re = (short)((r2*cos(5 * M_PI / 12.0))*CP);
			m_32apsk[1].im = (short)((r2*sin(5 * M_PI / 12.0))*CP);
			m_32apsk[2].re = (short)((r2*cos(-M_PI / 4.0))*CP);
			m_32apsk[2].im = (short)((r2*sin(-M_PI / 4.0))*CP);
			m_32apsk[3].re = (short)((r2*cos(-5 * M_PI / 12.0))*CP);
			m_32apsk[3].im = (short)((r2*sin(-5 * M_PI / 12.0))*CP);
			m_32apsk[4].re = (short)((r2*cos(3 * M_PI / 4.0))*CP);
			m_32apsk[4].im = (short)((r2*sin(3 * M_PI / 4.0))*CP);
			m_32apsk[5].re = (short)((r2*cos(7 * M_PI / 12.0))*CP);
			m_32apsk[5].im = (short)((r2*sin(7 * M_PI / 12.0))*CP);
			m_32apsk[6].re = (short)((r2*cos(-3 * M_PI / 4.0))*CP);
			m_32apsk[6].im = (short)((r2*sin(-3 * M_PI / 4.0))*CP);
			m_32apsk[7].re = (short)((r2*cos(-7 * M_PI / 12.0))*CP);
			m_32apsk[7].im = (short)((r2*sin(-7 * M_PI / 12.0))*CP);
			m_32apsk[8].re = (short)((r3*cos(M_PI / 8.0))*CP);
			m_32apsk[8].im = (short)((r3*sin(M_PI / 8.0))*CP);
			m_32apsk[9].re = (short)((r3*cos(3 * M_PI / 8.0))*CP);
			m_32apsk[9].im = (short)((r3*sin(3 * M_PI / 8.0))*CP);
			m_32apsk[10].re = (short)((r3*cos(-M_PI / 4.0))*CP);
			m_32apsk[10].im = (short)((r3*sin(-M_PI / 4.0))*CP);
			m_32apsk[11].re = (short)((r3*cos(-M_PI / 2.0))*CP);
			m_32apsk[11].im = (short)((r3*sin(-M_PI / 2.0))*CP);
			m_32apsk[12].re = (short)((r3*cos(3 * M_PI / 4.0))*CP);
			m_32apsk[12].im = (short)((r3*sin(3 * M_PI / 4.0))*CP);
			m_32apsk[13].re = (short)((r3*cos(M_PI / 2.0))*CP);
			m_32apsk[13].im = (short)((r3*sin(M_PI / 2.0))*CP);
			m_32apsk[14].re = (short)((r3*cos(-7 * M_PI / 8.0))*CP);
			m_32apsk[14].im = (short)((r3*sin(-7 * M_PI / 8.0))*CP);
			m_32apsk[15].re = (short)((r3*cos(-5 * M_PI / 8.0))*CP);
			m_32apsk[15].im = (short)((r3*sin(-5 * M_PI / 8.0))*CP);
			m_32apsk[16].re = (short)((r2*cos(M_PI / 12.0))*CP);
			m_32apsk[16].im = (short)((r2*sin(M_PI / 12.0))*CP);
			m_32apsk[17].re = (short)((r1*cos(M_PI / 4.0))*CP);
			m_32apsk[17].im = (short)((r1*sin(M_PI / 4.0))*CP);
			m_32apsk[18].re = (short)((r2*cos(-M_PI / 12.0))*CP);
			m_32apsk[18].im = (short)((r2*sin(-M_PI / 12.0))*CP);
			m_32apsk[19].re = (short)((r1*cos(-M_PI / 4.0))*CP);
			m_32apsk[19].im = (short)((r1*sin(-M_PI / 4.0))*CP);
			m_32apsk[20].re = (short)((r2*cos(11 * M_PI / 12.0))*CP);
			m_32apsk[20].im = (short)((r2*sin(11 * M_PI / 12.0))*CP);
			m_32apsk[21].re = (short)((r1*cos(3 * M_PI / 4.0))*CP);
			m_32apsk[21].im = (short)((r1*sin(3 * M_PI / 4.0))*CP);
			m_32apsk[22].re = (short)((r2*cos(-11 * M_PI / 12.0))*CP);
			m_32apsk[22].im = (short)((r2*sin(-11 * M_PI / 12.0))*CP);
			m_32apsk[23].re = (short)((r1*cos(-3 * M_PI / 4.0))*CP);
			m_32apsk[23].im = (short)((r1*sin(-3 * M_PI / 4.0))*CP);
			m_32apsk[24].re = (short)((r3*cos(0.0))*CP);
			m_32apsk[24].im = (short)((r3*sin(0.0))*CP);
			m_32apsk[25].re = (short)((r3*cos(M_PI / 4.0))*CP);
			m_32apsk[25].im = (short)((r3*sin(M_PI / 4.0))*CP);
			m_32apsk[26].re = (short)((r3*cos(-M_PI / 8.0))*CP);
			m_32apsk[26].im = (short)((r3*sin(-M_PI / 8.0))*CP);
			m_32apsk[27].re = (short)((r3*cos(-3 * M_PI / 8.0))*CP);
			m_32apsk[27].im = (short)((r3*sin(-3 * M_PI / 8.0))*CP);
			m_32apsk[28].re = (short)((r3*cos(7 * M_PI / 8.0))*CP);
			m_32apsk[28].im = (short)((r3*sin(7 * M_PI / 8.0))*CP);
			m_32apsk[29].re = (short)((r3*cos(5 * M_PI / 8.0))*CP);
			m_32apsk[29].im = (short)((r3*sin(5 * M_PI / 8.0))*CP);
			m_32apsk[30].re = (short)((r3*cos(M_PI))*CP);
			m_32apsk[30].im = (short)((r3*sin(M_PI))*CP);
			m_32apsk[31].re = (short)((r3*cos(-3 * M_PI / 4.0))*CP);
			m_32apsk[31].im = (short)((r3*sin(-3 * M_PI / 4.0))*CP);
		}
		// BPSK
		m_bpsk[0][0].re = (short)((r0*cos(1 * M_PI / 4.0))*CP);
		m_bpsk[0][0].im = (short)((r0*sin(1 * M_PI / 4.0))*CP);
		m_bpsk[0][1].re = (short)((r0*cos(5 * M_PI / 4.0))*CP);
		m_bpsk[0][1].im = (short)((r0*sin(5 * M_PI / 4.0))*CP);

		m_bpsk[1][0].re = (short)((r0*cos(3 * M_PI / 4.0))*CP);
		m_bpsk[1][0].im = (short)((r0*sin(3 * M_PI / 4.0))*CP);
		m_bpsk[1][1].re = (short)((r0*cos(7 * M_PI / 4.0))*CP);
		m_bpsk[1][1].im = (short)((r0*sin(7 * M_PI / 4.0))*CP);

}
