/* Fenix1parser.hh - Parse Siemens Fenix1 frames
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

#ifndef FENIX1PARSER_HH
#define FENIX1PARSER_HH

#include "XR25streamreader.hh"

// This parser is not tested; if you test it, report feedback!
class Fenix1parser : public XR25frameparser {
public:
	virtual inline bool parse_frame(const unsigned char c[], int length,
					XR25frame &fra) override {
		fra.program_vrsn  = c[2];
		fra.calib_vrsn    = c[3];
		fra.in_flags      = remap_bit(c[4],   0x02, IN_PARKED)
				    | remap_bit(c[4], 0x04, IN_AC_REQUEST)
				    | remap_bit(c[4], 0x08, IN_THROTTLE_0)
				    | remap_bit(c[4], 0x10, IN_THROTTLE_1)
				    | remap_bit(c[4], 0x20, IN_AC_COMPRES);
		// fra.out_flags     not sent?;
		fra.map           = 4 * c[5];
		fra.rpm           = ({ int tmp = (c[11] << 8) | c[10];
				 tmp ? (0x00e4e1c0 / tmp) : 0; });
		fra.throttle      = c[22] / 2.55;
		fra.fault_flags_1 = c[19];
		fra.eng_pinging   = c[14];
		fra.injection_us  = 2 * ((c[13] << 8) | c[12]);
		fra.advance       = c[15];
		fra.fault_flags_0 = c[27];
		fra.fault_fugitive = c[26];
		fra.fault_flags_2 = c[18];
		fra.temp_water    = (c[6] / 1.6) - 40;
		fra.temp_air      = (c[7] / 1.6) - 40;
		fra.batt_v        = (c[8] / 32.0) + 8;
		// fra.lambda_v      = 6 * c[?];
		fra.idle_regulation = c[16] / 2.55;
		fra.idle_period     = c[21];
		fra.eng_pinging_delay = c[28];
		fra.atmos_pressure  = 4 * (~c[29] & 0xff);
		fra.spd_km_h      = c[20];
		
		return            (length > 29);
	}
};

#endif /* FENIX1PARSER_HH */
