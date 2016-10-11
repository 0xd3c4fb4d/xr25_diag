/* Fenix52Bparser.hh - Parse Fenix 52 bytes frame (Renault r21 2.0TXi) frames
 *
 * Copyright (C) Javier L. Gómez, 2016
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef FENIX52BPARSER_HH
#define FENIX52BPARSER_HH

#include "XR25streamreader.hh"

// This parser is incomplete; if you know the meaning of any other byte,
// please contribute!
class Fenix52Bparser : public XR25frameparser {
public:
	virtual inline bool parse_frame(const unsigned char c[], int length,
					XR25frame &fra) override {
		fra.program_vrsn  = c[2];
		fra.calib_vrsn    = c[3];
		fra.in_flags      = remap_bit(c[6],   0x80, IN_THROTTLE_0)
			            | remap_bit(c[6], 0x40, IN_THROTTLE_1);
		fra.out_flags     = remap_bit(c[5],   0x80, OUT_LAMBDA_LOOP);
		fra.map           = 4 * c[24];
		fra.rpm           = ({ int tmp = (c[20] << 8) | c[19];
				 tmp ? (0x00e4e1c0 / tmp) : 0; });
		fra.throttle      = c[25] / 2.55;
		// fra.fault_flags_1 = c[?];
		fra.eng_pinging   = c[31];
		// fra.injection_us  = 2 * ((c[?] << 8) | c[?]);
		// fra.advance       = c[?];
		// fra.fault_flags_0 = c[?];
		// fra.fault_fugitive = c[?];
		// fra.fault_flags_2 = c[?];
		// fra.fault_flags_4 = c[?];
		// fra.fault_flags_3 = c[?];
		fra.temp_water    = (c[27] / 1.6) - 40;
		fra.temp_air      = (c[28] / 1.6) - 40;
		fra.batt_v        = (c[29] / 32.0) + 8;
		fra.lambda_v      = 6 * c[26];
		// fra.idle_regulation = c[?] / 2.55;
		// fra.idle_period     = c[?];
		// fra.eng_pinging_delay = c[?];
		// fra.atmos_pressure  = 4 * (~c[?] & 0xff);
		// fra.afr_correction  = c[?];
		fra.spd_km_h      = c[30];
		
		return            (length == 52);
	}
};

#endif /* FENIX52BPARSER_HH */
