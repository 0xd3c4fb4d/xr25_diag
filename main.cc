/* main.cc - xr25_diag GPL Renault XR25 diagnostic software
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

#include <cstdlib>
#include <cstring>
#include <regex>
#include <gtkmm.h>
#include <asm/termbits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include "XR25streamreader.hh"
#include "ParserFactory.hh"
#include "UI.hh"
#include "tee_stdio_filebuf.hh"

struct ParamsStruct {
	Glib::ustring dev_path;    /* tty device path */
	Glib::ustring parser_t;    /* parser class typename */
	Glib::ustring tty_conf;    /* serial port configuration; string format:
				    * <baud>,<character size><parity><stop bits>
				    * e.g., 62500,8N1 */
	Glib::ustring save_pathname;   /* pathname of a file to write received
					* frames to */
};

/** Get port configuration from user.
 * @param b Gtk::Builder object to use; 'conf_dialog' is a GtkDialog req-
 *     uesting configuration from user
 * @param params Returned parameters struct
 */
bool get_port_conf(Glib::RefPtr<Gtk::Builder> b, ParamsStruct &params) {
#define DEV_PATH "/dev/"
	Gtk::Dialog *conf_dialog;
	Gtk::ComboBoxText *dev_path, *parser_t;
	Gtk::Entry        *tty_conf, *save_pathname;
	Gtk::Button       *save_as;
	b->get_widget("conf_dialog", conf_dialog);
	b->get_widget("cd_dev_path", dev_path);
	b->get_widget("cd_parser_t", parser_t);
	b->get_widget("cd_tty_conf", tty_conf);
	b->get_widget("cd_save_pathname", save_pathname);
	b->get_widget("cd_save_as",       save_as);
	
	DIR *dirp = opendir(DEV_PATH);
	struct dirent *dirent;
	for (std::regex re("tty(S|ACM|USB)[0-9]+|stdin");
	     (dirent = readdir(dirp)) || closedir(dirp); ) {
		std::string d_name(dirent->d_name);
		if (std::regex_match(d_name, re))
			dev_path->append(DEV_PATH + d_name);
	}
	dev_path->set_active(0);

	for (auto &i : ParserFactory::get_registered_types())
		parser_t->append(i.first);
	parser_t->set_active(0);

	save_as->signal_clicked().connect([conf_dialog,save_pathname,&params](){
			Gtk::FileChooserDialog _d(*conf_dialog, "Write file:",
						 Gtk::FILE_CHOOSER_ACTION_SAVE);
			_d.add_button("Cancel", Gtk::RESPONSE_CANCEL);
			_d.add_button("OK", Gtk::RESPONSE_OK);
			_d.set_do_overwrite_confirmation();
			if (_d.run() == Gtk::RESPONSE_OK)
				save_pathname->set_text(_d.get_filename());
		});
	int ret = conf_dialog->run();
	conf_dialog->hide();

	return ({ params.dev_path = dev_path->get_active_text();
		params.parser_t = parser_t->get_active_text();
		params.tty_conf = tty_conf->get_text();
		params.save_pathname = save_pathname->get_text(); })
		, ret == Gtk::RESPONSE_OK;
}

/** Serial port setup.
 * @param fd File descriptor
 * @param conf Serial port configuration; ignored, defaults to &quot;
 *     62500,8N1&quot;
 */
void ttyS_init(int fd, Glib::ustring conf) {
	struct termios2 t_io = { 0, 0, CREAD | BOTHER | CS8,
				 0, 0, { }, 62500, 62500 };
	t_io.c_cc[VMIN] = 1;
	ioctl(fd, TCSETS2, &t_io);
	// O_NDELAY open() flag disables blocking mode for I/O; reenable
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
}

int main(int argc, char *argv[]) {
	auto application = Gtk::Application::create(argc, argv,
						    "com.github.xr25_diag");
	Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file
		("xr25_diag.glade");
	ParamsStruct params;
	std::filebuf ob;
	
	if (!get_port_conf(builder, params))
		return EXIT_SUCCESS;

	int fd = open(params.dev_path.c_str(), O_RDWR | O_NOCTTY
		      | O_NDELAY /* don't wait DCD signal */);
	if (fd == -1) {
		const char *err_str = g_strerror(errno);
		Gtk::MessageDialog e("open() " + params.dev_path + " failed",
				     /* use_markup= */ 0, Gtk::MESSAGE_ERROR);
		e.set_secondary_text(err_str), e.run();
		return EXIT_FAILURE;
	}
	ttyS_init(fd, params.tty_conf);

	if (!params.save_pathname.empty())
		ob.open(params.save_pathname, std::ios_base::out);
	std::unique_ptr<__gnu_cxx::stdio_filebuf<char> > filebuf(ob.is_open()
		   ? new tee_stdio_filebuf<char>(fd, std::ios_base::in, ob)
		   : new __gnu_cxx::stdio_filebuf<char>(fd, std::ios_base::in));
	std::istream is(filebuf.get());

	UI(application, builder, is, *ParserFactory::create(params.parser_t))
		.run();
	return EXIT_SUCCESS;
}
