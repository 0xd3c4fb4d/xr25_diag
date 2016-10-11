/* ParserFactory.hh - instantiate objects of type 'XXXparser'
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

#ifndef PARSERFACTORY_HH
#define PARSERFACTORY_HH

#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include "XR25streamreader.hh"
#include "Fenix3parser.hh"
#include "Fenix52Bparser.hh"

class ParserFactory {
public:
	typedef std::shared_ptr<XR25frameparser> parser_ptr_t;
private:
	typedef std::map<std::string,
			 std::function<parser_ptr_t()> > ctor_map_t;

	static const ctor_map_t __map;
public:
	static parser_ptr_t create(const std::string &_typename)
	{ return __map.at(_typename)(); }
	static const ctor_map_t &get_registered_types() { return __map; }
};

#define REGISTER_TYPE(_typename) { #_typename, []() {     \
		return std::make_shared<_typename>(); } }

/** Use the REGISTER_TYPE(xxx) macro to add new parser types here.
 */
const ParserFactory::ctor_map_t ParserFactory::__map = {
	REGISTER_TYPE(Fenix3parser),
	REGISTER_TYPE(Fenix52Bparser),
};

#endif /* PARSERFACTORY_HH */
