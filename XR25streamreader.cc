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
#include <condition_variable>
#include <tuple>
#include <mutex>

/** Frame received handler.
 * @param parser The XR25frameparser to use
 * @param c Translated frame (&quot;0xff 0xff&quot; replaced by &quot;0xff
 *     &quot;)
 * @param length Length of the frame in octets
 * @param fra Reference to the parsed frame
 */
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
	std::condition_variable term;
	std::mutex              term_m;
	std::atomic_int         count(0);
	// thread that updates __fra_sec once a second
	std::thread stat_thread([&term, &term_m, &count, this]() {
			std::unique_lock<std::mutex> lock(term_m);
			while (term.wait_for(lock, std::chrono::seconds(1))
			       == std::cv_status::timeout)
				this->__fra_sec = count.exchange(0); });
	
	// thread cancellation clean-up handler
	typedef std::tuple<std::condition_variable&, std::thread &> arg_tuple_t;
	auto args = std::make_tuple(std::ref(term), std::ref(stat_thread));
	pthread_cleanup_push([](void *p) {
			auto args = *static_cast<arg_tuple_t *>(p);
			std::get<0>(args).notify_one();
			std::get<1>(args).join();
		}, &args);
	
	while (!__in.eof()) {
		if ((c = __in.get()) == 0xff) {
			if ((c = __in.get()) == 0x00) { /* start of frame */
				if (__synchronized)
					frame_recv(parser, frame, p - frame,
						   fra), count++;
				__synchronized = 1, p = &frame[1];
			} else if (c != 0xff) /* translate 'ff ff' to 'ff' */
				__in.unget();
		}

		if (__synchronized)
			(p - frame) < ARRAY_SIZE(frame) ? *p++ = c
				      : __synchronized = 0, __sync_err_count++;
	}
	pthread_cleanup_pop(1);
}
