/* tee_stdio_filebuf.hh - a stdio_filebuf that behaves as tee(1)
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

#include <ext/stdio_filebuf.h>

template<typename _CharT, typename _Traits = std::char_traits<_CharT> >
class tee_stdio_filebuf : public __gnu_cxx::stdio_filebuf<_CharT, _Traits> {
protected:
	typedef __gnu_cxx::stdio_filebuf<_CharT, _Traits> filebuf_type;
	std::ostream __out;

	typename filebuf_type::int_type underflow() {
		auto ret = filebuf_type::underflow();
		__out.write(filebuf_type::gptr(), filebuf_type::egptr()
			                          - filebuf_type::gptr());
		return ret;
	}

public:
	/**
	 *  @param  __fd  An open file descriptor.
	 *  @param  __mode  Same meaning as in a standard filebuf.
	 *  @param  __obuf The std::basic_filebuf to write if a read from this
	 *      filebuf is attempted
	 *
	 *  This constructor associates a file stream buffer with an open
	 *  POSIX file descriptor; a read from this filebuf will cause the read
	 *  buffer to be written to __obuf. The file descriptor will be
	 *  automatically closed when the stdio_filebuf is closed/destroyed.
	 */
	tee_stdio_filebuf(int __fd, std::ios_base::openmode __mode,
			  std::basic_filebuf<_CharT, _Traits> &__obuf)
		: filebuf_type(__fd, __mode), __out(&__obuf) { }
};
