/* UI.hh - xr25_diag user interface (gtkmm)
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

#include <gtkmm.h>

#define UI_UPDATE_HZ 16
class UI {
private:
	Glib::RefPtr<Gtk::Application> __application;
	Glib::RefPtr<Gtk::Builder>     __builder;
	XR25streamreader               __xr25reader;
	const XR25frameparser          &__fp;

	Gtk::Window *__main_window;

	bool update() { return TRUE;	}
public:
	UI(Glib::RefPtr<Gtk::Application> _a, Glib::RefPtr<Gtk::Builder> _b,
	   std::istream &_is, const XR25frameparser &_p)
		: __application(_a), __builder(_b), __xr25reader(_is),
		  __fp(_p) {
		__builder->get_widget("main_window", __main_window);
	}
	~UI() { __xr25reader.stop(); }
	
	void run() {
		Glib::signal_timeout().connect(sigc::mem_fun(*this, &UI::update)
					       , 1000 / UI_UPDATE_HZ);
		__application->run(*__main_window);
	}
};
