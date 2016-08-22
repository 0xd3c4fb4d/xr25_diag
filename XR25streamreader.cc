/* XR25streamreader.cc - Parse Renault XR25 frame stream
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

#include "XR25streamreader.hh"

void XR25streamreader::frame_recv(XR25frameparser &parser,
				  const unsigned char c[], int length,
				  XR25frame &fra) {
	parser.parse_frame(c, length, fra);
	if (__post_parse)
		__post_parse(c, length, fra);
}

void XR25streamreader::read_frames(XR25frameparser &parser) {
	unsigned char frame[128] = { 0xff, 0x00 }, c, *p = &frame[1];
	XR25frame fra;
	while (!__in.eof()) {
		if ((c = __in.get()) == 0xff) {
			if ((c = __in.get()) == 0x00) { /* start of frame */
				if (__synchronized)
					frame_recv(parser, frame, p - frame,
						   fra);
				__synchronized = 1, p = &frame[1];
			} else if (c != 0xff) /* translate 'ff ff' to 'ff' */
				__in.unget();
		}

		if (__synchronized)
			(p - frame) < ARRAY_SIZE(frame) ? *p++ = c
				      : __synchronized = 0;
	}
}
