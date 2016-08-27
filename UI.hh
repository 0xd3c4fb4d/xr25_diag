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

#ifndef UI_HH
#define UI_HH

#include <mutex>
#include <gtkmm.h>
#include <pangomm/context.h>
#include "XR25streamreader.hh"
#include "CairoGauge.hh"

class UI {
private:
	Glib::RefPtr<Gtk::Application> __application;
	Glib::RefPtr<Gtk::Builder>     __builder;
	XR25streamreader               __xr25reader;
	const XR25frameparser          &__fp;

	XR25frame  __last_recv;
	std::mutex __last_recv_mutex;

	Gtk::Label    *__hb_sync_err, *__hb_fra_s;
	Gtk::Image    *__hb_is_sync;
	Gtk::Notebook *__notebook;

	enum EntryWidgets {
		E_PROGRAM_VRSN = 0,
		E_CALIB_VRSN,
		E_MAP,
		E_RPM,
		E_THROTTLE,
		E_ENG_PINGING,      /* 5  */
		E_INJECTION_US,
		E_ADVANCE,
		E_TEMP_WATER,
		E_TEMP_AIR,
		E_BATT_V,           /* 10 */
		E_LAMBDA_V,
		E_IDLE_REGULATION,
		E_IDLE_PERIOD,
		E_ENG_PINGING_DELAY,
		E_ATMOS_PRESSURE,   /* 15 */
		E_AFR_CORRECTION,
		E_SPD_KM_H,
		E_COUNT,            // add new elements before this line
	};
	enum FlagWidgets {
		F_IN_AC_REQUEST = 0,
		F_IN_AC_COMPRES,
		F_IN_THROTTLE_0,
		F_IN_PARKED,
		F_IN_THROTTLE_1,
		F_OUT_PUMP_ENABLE,       /* 5  */
		F_OUT_IDLE_REGULATION,
		F_OUT_WASTEGATE_REG,
		F_OUT_EGR_ENABLE,
		F_OUT_CHECK_ENGINE,
		F_FAULT_MAP,             /* 10 */
		F_FAULT_SPD_SENSOR,
		F_FAULT_LAMBDA_TMP,
		F_FAULT_LAMBDA,
		F_FAULT_WATER_OPEN_C,
		F_FAULT_WATER_SHORT_C,   /* 15 */
		F_FAULT_AIR_OPEN_C,
		F_FAULT_AIR_SHORT_C,
		F_FAULT_TPS_LOW,
		F_FAULT_TPS_HIGH,
		F_FAULT_F_WATER_OPEN_C,  /* 20 */
		F_FAULT_F_WATER_SHORT_C,
		F_FAULT_F_AIR_OPEN_C,
		F_FAULT_F_AIR_SHORT_C,
		F_FAULT_F_TPS_LOW,
		F_FAULT_F_TPS_HIGH,      /* 25 */
		F_FAULT_EEPROM_CHECKSUM,
		F_FAULT_PROG_CHECKSUM,
		F_FAULT_PUMP,
		F_FAULT_WASTEGATE,
		F_FAULT_EGR,             /* 30 */
		F_FAULT_IDLE_REG,
		F_FAULT_INJECTORS,
		F_COUNT,                 // add new elements before this line
	};

	Gtk::Entry    *__entry[E_COUNT];
	Gtk::Arrow    *__flag[F_COUNT];

	enum { G_RPM = 0, G_SPD_KM_H, G_TEMP_WATER, G_BATT_V, G_COUNT };
	CairoGauge     __gauge[G_COUNT] = {
		{ "RPM", [](void *p) { return static_cast<XR25frame*>(p)->rpm; }
		  , 7000, 1000 },
		{ "km/h", [](void *p) { return static_cast<XR25frame*>(p)
					->spd_km_h; }, 240, 20 },
		{ "Temp (C)", [](void *p) { return static_cast<XR25frame*>(p)
					    ->temp_water; }, 120, 30 },
		{ "Battery (V)", [](void *p) { return static_cast<XR25frame*>(p)
					       ->batt_v; }, 18, 2 }
	};
	Glib::RefPtr<Gtk::TextBuffer>    __dash_tb;

	void update_page_diagnostic(XR25frame &);
	void update_page_dashboard(XR25frame &);

	/** Update current notebook page, see 'update_page_xxx()' member
	 * functions; called UI_UPDATE_PAGE_HZ times per sec.
	 */
	bool update_page() {
		sigc::bound_mem_functor1<void, UI, XR25frame&> _fn[] = {
			sigc::mem_fun(*this, &UI::update_page_diagnostic),
			sigc::mem_fun(*this, &UI::update_page_dashboard),
		};

		__last_recv_mutex.lock();
		XR25frame fra = __last_recv;
		__last_recv_mutex.unlock();

		_fn[__notebook->get_current_page()](fra);
		return TRUE;
	}
	
	/** Update headerbar widgets; called UI_UPDATE_HEADER_HZ times per sec
	 */
	bool update_header() {
		__hb_sync_err->set_text(std::to_string(__xr25reader
						       .get_sync_err_count()));
		__hb_fra_s->set_text(std::to_string(__xr25reader
						    .get_fra_per_sec()));
		__hb_is_sync->set_from_icon_name(__xr25reader.is_synchronized()
						 ? "gtk-yes" : "gtk-no",
						 Gtk::ICON_SIZE_BUTTON);
		return TRUE;
	}
public:
	UI(Glib::RefPtr<Gtk::Application> _a, Glib::RefPtr<Gtk::Builder> _b,
	   std::istream &_is, const XR25frameparser &_p)
		: __application(_a), __builder(_b),
		  __xr25reader(_is, [this](const unsigned char c[], int l,
					   XR25frame &fra) {
				       this->__last_recv_mutex.lock();
				       this->__last_recv = fra; // copy struct
				       this->__last_recv_mutex.unlock();
			       }),  __fp(_p) {
		__builder->get_widget("mw_hb_sync_err", __hb_sync_err);
		__builder->get_widget("mw_hb_fra_s",    __hb_fra_s);
		__builder->get_widget("mw_hb_is_sync",  __hb_is_sync);
		__builder->get_widget("mw_notebook",    __notebook);
		
		for (int i = 0; i < E_COUNT; i++)
			__builder->get_widget("mw_e" + std::to_string(i),
					      __entry[i]);
		for (int i = 0; i < F_COUNT; i++)
			__builder->get_widget("mw_f" + std::to_string(i),
					      __flag[i]);
	}
	~UI() { __xr25reader.stop(); }
	
#define UI_UPDATE_PAGE_HZ   16
#define UI_UPDATE_HEADER_HZ 1
	void run() {
		Gtk::Grid *dash_grid = nullptr;
		__builder->get_widget("mw_dash_grid", dash_grid);
		dash_grid->attach(__gauge[G_RPM],        0, 0, 1, 1);
		dash_grid->attach(__gauge[G_SPD_KM_H],   2, 0, 1, 1);
		dash_grid->attach(__gauge[G_TEMP_WATER], 0, 1, 1, 1);
		dash_grid->attach(__gauge[G_BATT_V],     2, 1, 1, 1);
		dash_grid->show_all();

		Gtk::TextView *dash_text = nullptr;
		__builder->get_widget("mw_dash_text", dash_text);
		dash_text->set_buffer((__dash_tb = Gtk::TextBuffer::create()));

		/* connect signals */
		Glib::signal_timeout().connect(sigc::mem_fun(*this,
							     &UI::update_page)
					       , 1000 / UI_UPDATE_PAGE_HZ);
		Glib::signal_timeout().connect(sigc::mem_fun(*this,
							     &UI::update_header)
					       , 1000 / UI_UPDATE_HEADER_HZ);
		Gtk::Button *about_button = nullptr;
		__builder->get_widget("mw_about_button", about_button);
		about_button->signal_clicked().connect([this]() {
					Gtk::AboutDialog *ad = nullptr;
					__builder->get_widget("about_dialog",
							      ad);
					ad->set_version(XR25DIAG_VERSION);
					ad->run(), ad->hide();
				});
		Gtk::CheckButton *hud = nullptr;
		__builder->get_widget("mw_hud", hud);
		hud->signal_toggled().connect([hud,dash_text,this]() {
				auto m = hud->get_active()
					? Cairo::Matrix{ -1, 0, 0, -1, 0, 0 }
					: Cairo::identity_matrix();
				for (auto &i : __gauge)
					i.set_transform_matrix(m);
				dash_text->get_pango_context()
					->set_matrix(hud->get_active()
						     ? Pango::Matrix{ -1,  0,
								       0, -1,
								       0,  0 }
						     : Pango::Matrix{ 1, 0,
								      0, 1,
								      0, 0 });
			});
		
		Gtk::Window *main_window  = nullptr;
		__builder->get_widget("main_window",     main_window);
		__application->run(*main_window);
	}
};

#endif /* UI_HH */
