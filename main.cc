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
#include <ext/stdio_filebuf.h>
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
#include "Fenix3parser.hh"
#include "UI.hh"

/** Get port configuration from user.
 * @param b Gtk::Builder object to use; 'conf_dialog' is a GtkDialog req-
 *     uesting configuration from user
 * @param path Returned &quot;/dev/ttyX&quot; path
 */
bool get_port_conf(Glib::RefPtr<Gtk::Builder> b, Glib::ustring &path) {
#define DEV_PATH "/dev/"
	Gtk::Dialog *conf_dialog;
	Gtk::ComboBoxText *dev_path;
	b->get_widget("conf_dialog", conf_dialog);
	b->get_widget("cd_dev_path", dev_path);
	
	DIR *dirp = opendir(DEV_PATH);
	struct dirent *dirent;
	for (std::regex re("tty(S|ACM|USB)[0-9]+");
	     (dirent = readdir(dirp)) || closedir(dirp); ) {
		std::string d_name(dirent->d_name);
		if (std::regex_match(d_name, re))
			dev_path->append(DEV_PATH + d_name);
	}
	dev_path->set_active(0);

	int ret = conf_dialog->run();
	conf_dialog->hide();
	return (path = dev_path->get_active_text()),
		ret == Gtk::RESPONSE_OK;
}

/** Serial port setup. 62500,8N1
 * @param fd File descriptor
 */
void ttyS_init(int fd) {
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
	Glib::ustring path;

	if (!get_port_conf(builder, path))
		return EXIT_SUCCESS;

	int fd = open(path.c_str(), O_RDWR | O_NOCTTY
		      | O_NDELAY /* don't wait DCD signal */);
	if (fd == -1) {
		const char *err_str = g_strerror(errno);
		Gtk::MessageDialog e("open() failed", /* use_markup */ 0,
				     Gtk::MESSAGE_ERROR);
		e.set_secondary_text(err_str), e.run();
		return EXIT_FAILURE;
	}
	ttyS_init(fd);
	
	__gnu_cxx::stdio_filebuf<char> filebuf(fd, std::ios_base::in);
	std::istream is(&filebuf);

	UI(application, builder, is, Fenix3parser()).run();
	close(fd);
	return EXIT_SUCCESS;
}
