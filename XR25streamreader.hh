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
#include <atomic>
#include <functional>
#include <pthread.h>
#include <thread>

/* XR25 frames start with 0xff 0x00; 0xff ocurrences in the frame sent on the
 * wire as 0xff 0xff.
 *
 * 0xff 0x00 0xaa 0xbb 0xcc 0xdd  .  .  .  0xff 0x00  .  .  .
 * |-------|   |   |    |    |---- byte 3  |-------|
 *  header1    |   |    |--------- byte 2   header2
 *             |   |-------------- byte 1
 *             |------------------ byte 0
 */
struct XR25frame {
	unsigned char program_vrsn; /* byte 0 */
	unsigned char calib_vrsn;   /* byte 1 */

	unsigned char in_flags;     /* byte 2 */
#define IN_AC_REQUEST 0x02
#define IN_AC_COMPRES 0x04
#define IN_THROTTLE_0 0x08
#define IN_PARKED 0x10
#define IN_THROTTLE_1 0x20

	unsigned char out_flags;    /* byte 3 */
#define OUT_PUMP_ENABLE     0x01
#define OUT_IDLE_REGULATION 0x02
#define OUT_WASTEGATE_REG   0x04
#define OUT_LAMBDA_LOOP     0x08
#define OUT_EGR_ENABLE      0x20
#define OUT_CHECK_ENGINE    0x80
	
	int map;                    /* byte 4 */
	int rpm;                    /* byte 5-6 */
	int throttle;               /* byte 7 */
	
	unsigned char fault_flags_1; /* byte 8 */
#define FAULT_MAP           0x04
#define FAULT_SPD_SENSOR    0x10
#define FAULT_LAMBDA_TMP    0x20
#define FAULT_LAMBDA        0x80

	unsigned char eng_pinging;   /* byte 9 */
	int injection_us;            /* byte 10-11 */
	int advance;                 /* byte 12 */
	
	unsigned char fault_flags_0; /* byte 14 */
#define FAULT_WATER_OPEN_C  0x01
#define FAULT_WATER_SHORT_C 0x02
#define FAULT_AIR_OPEN_C    0x04
#define FAULT_AIR_SHORT_C   0x08
#define FAULT_TPS_LOW       0x40
#define FAULT_TPS_HIGH      0x80
	
	unsigned char fault_fugitive; /* byte 15, see 'fault_flags_0' bits */
	
	unsigned char fault_flags_2; /* byte 16 */
#define FAULT_EEPROM_CHECKSUM 0x20
#define FAULT_PROG_CHECKSUM   0x80
	
	unsigned char fault_flags_4; /* byte 17 */
#define FAULT_PUMP     0x01
#define FAULT_WASTEGATE 0x04
#define FAULT_EGR      0x08
#define FAULT_IDLE_REG 0x20
	
	unsigned char fault_flags_3; /* byte 18 */
#define FAULT_INJECTORS    0x10

	float temp_water;            /* byte 19 */
	float temp_air;              /* byte 20 */
	float batt_v;                /* byte 21 */
	float lambda_v;              /* byte 22 */
	int idle_regulation;         /* byte 23 */
	int idle_period;             /* byte 24 */
	unsigned char eng_pinging_delay; /* byte 25 */
	int atmos_pressure;          /* byte 26 */
	unsigned char afr_correction;    /* byte 28 */
	int spd_km_h;                /* byte 32 */
};

class XR25frameparser {
public:
	/** Parses a frame and return a 'struct XR25frame'.
	 * @param c Translated frame (&quot;0xff 0xff&quot; converted to 0xff)
	 * @param length Length in octets
	 * @param fra Reference to 'struct XR25frame'; implementors write here
	 * @return true if the frame @a c was parsed
	 */
	virtual bool parse_frame(const unsigned char c[], int length,
		XR25frame &fra) = 0;
};

#define ARRAY_SIZE(_a) (unsigned int)(sizeof(_a) / sizeof(_a[0]))
	
class XR25streamreader {
private:
	typedef std::function<void(const unsigned char[], int, XR25frame &)
			      > post_parse_t;

	std::istream     &__in;
	std::atomic_bool __synchronized;
	std::atomic_int  __sync_err_count, __fra_sec;
	post_parse_t     __post_parse;
	std::thread      *__thrd;
	
	void frame_recv(XR25frameparser &parser, const unsigned char[], int
		, XR25frame &);
	void read_frames(XR25frameparser &parser);
public:
	XR25streamreader(std::istream &s, post_parse_t p = nullptr)
		: __in(s), __synchronized(0),
		  __sync_err_count(0), __fra_sec(0), __post_parse(p),
		  __thrd(nullptr) {}
	~XR25streamreader() { stop(); }

	bool is_synchronized() { return __synchronized.load(); }
	int  get_sync_err_count() { return __sync_err_count.load(); }
	int  get_fra_per_sec() { return __fra_sec.load(); }
	
	/** Read frames non-blocking; call stop() to cancel thread
	 * @param parser The XR25frameparser to use
	 */
	void start(XR25frameparser &parser) {
		if (!__thrd)
			__thrd = new std::thread([&parser, this]() {
					this->read_frames(parser); });
	}
	
	/** Stop internal thread; see start()
	 */
	inline void stop() {
		if (__thrd) {
			pthread_cancel(__thrd->native_handle()), __thrd->join();
			delete __thrd;
		}
	}
};

#endif /* XR25STREAMREADER_HH */
