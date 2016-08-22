/* XR25streamreader.hh - Parse Renault XR25 frame stream
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

#ifndef XR25STREAMREADER_HH
#define XR25STREAMREADER_HH

#include <iostream>

/* 0xff 0x00 0xaa 0xbb 0xcc 0xdd  .  .  .  0xff 0x00
 * |-------|   |   |    |    |---- byte 3
 *  header     |   |    |--------- byte 2
 *             |   |-------------- byte 1
 *             |------------------ byte 0
 */
struct XR25frame {
	unsigned char program_vrsn; /* byte 0 */
	unsigned char calib_vrsn;   /* byte 1 */

	unsigned char in_flags;     /* byte 2 */
	
	unsigned char out_flags;    /* byte 3 */

	int map;                    /* byte 4 */
	int rpm;                    /* byte 5 */
};

class XR25frameparser {
public:
	virtual bool parse_frame(const unsigned char c[], int length,
		XR25frame &fra) = 0;
};

#define ARRAY_SIZE(_a) (unsigned int)(sizeof(_a) / sizeof(_a[0]))
	
class XR25streamreader {
private:
	typedef void (*post_parse_t)(const unsigned char[], int, XR25frame &);

	std::istream &__in;
	bool          __synchronized;
	post_parse_t  __post_parse;
	void frame_recv(XR25frameparser &parser, const unsigned char[], int
		, XR25frame &);
public:
	XR25streamreader(std::istream &s, post_parse_t p = nullptr)
		: __in(s), __synchronized(0), __post_parse(p) {}
	void read_frames(XR25frameparser &parser);
};

#endif /* XR25STREAMREADER_HH */
