/* Fenix3parser.hh - Parse Fenix3 (Renault r21) frames
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

#ifndef FENIX3PARSER_HH
#define FENIX3PARSER_HH

#include "XR25streamreader.hh"

class Fenix3parser : public XR25frameparser {
public:
	virtual inline bool parse_frame(const unsigned char c[], int length,
					XR25frame &fra) override {
		fra.program_vrsn  = c[2];
		fra.calib_vrsn    = c[3];
		fra.in_flags      = c[4];
		fra.out_flags     = c[5];
		fra.map           = 4 * c[6];
		fra.rpm           = ({ int tmp = (c[8] << 8) | c[7];
				 tmp ? (0x00e4e1c0 / tmp) : 0; });
		fra.throttle      = c[9] / 2.55;
		fra.fault_flags_1 = c[10];
		fra.eng_pinging   = c[11];
		fra.injection_us  = 2 * ((c[13] << 8) | c[12]);
		fra.advance       = c[14];
		fra.fault_flags_0 = c[16];
		fra.fault_fugitive = c[17];
		fra.fault_flags_2 = c[18];
		fra.fault_flags_4 = c[19];
		fra.fault_flags_3 = c[20];
		fra.temp_water    = (c[21] / 1.6) - 40;
		fra.temp_air      = (c[22] / 1.6) - 40;
		fra.batt_v        = (c[23] / 32.0) + 8;
		fra.lambda_v      = 6 * c[24];
		fra.idle_regulation = c[25] / 2.55;
		fra.idle_period     = c[26];
		fra.eng_pinging_delay = c[27];
		fra.atmos_pressure  = 4 * (~c[28] & 0xff);
		fra.afr_correction  = c[30];
		fra.spd_km_h      = c[34];
		
		return            (length > 34);
	}
};

#endif /* FENIX3PARSER_HH */
