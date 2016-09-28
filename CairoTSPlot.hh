/* CairoTSPlot.hh - a time-series plot widget
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

#ifndef CAIROTSPLOT_HH
#define CAIROTSPLOT_HH

#include <string>
#include <functional>
#include <chrono>
#include <cmath>
#include <atomic>
#include <gtkmm.h>
#include <cairomm/context.h>

#define CAIROTSPLOT_FONT_SIZE 14
#define MARGIN_LEFT         4
#define MARGIN_TOP          40
#define MARGIN_RIGHT        32
#define MARGIN_BOTTOM       32
#define __RGBA_DEFAULT      Gdk::RGBA{"#2e7db3"}
#define __RGBA_ALERT        Gdk::RGBA{"#cc0d29"}
//  NUM_POINTS should be a power-of-2
#define NUM_POINTS          512

class CairoTSPlot : public Gtk::DrawingArea {
protected:
	typedef std::function<double(void *, bool&)> sample_fn_t;
	struct value_struct {
		double _v;
		bool _alert;
		bool _has_tp;
		std::chrono::time_point<
			std::chrono::steady_clock> _tp;
		value_struct() : _v(HUGE_VAL), _alert(FALSE), _has_tp(FALSE) { }
	};
	
	std::string  __text;
	sample_fn_t  __sample_fn;
	std::unique_ptr<value_struct[]> __data;
	std::atomic_uint                __data_i;
	std::atomic_bool              __data_chg;
	std::chrono::time_point<std::chrono::steady_clock> __last_tp;
	double       __value_min, __value_max,
		__tick_step, __data_height;
	Gdk::RGBA       __text_rgba;
	Cairo::Matrix __transform_matrix;
	Cairo::RefPtr<Cairo::Surface> __background;

	void draw_background(void);
	
	// Override Gtk::DrawingArea::on_draw() signal handler
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cc) override;
	// Override Gtk::Widget::on_size_allocate() signal handler
	void on_size_allocate(Gtk::Allocation& allocation) override {
		Gtk::Widget::on_size_allocate(allocation);
		__data_height = allocation.get_height() - MARGIN_TOP
			                                - MARGIN_BOTTOM;
		if (__background)
			draw_background();
	}
public:
	/** Construct a CairoTSPlot object
	 * @param text Text rendered above the plot
	 * @param fn std::function<double(void *, bool&)> that returns the 
	 *     next value; the second argument is used to change the _alert
	 *     member of the current 'struct value_struct'
	 * @param _m Minimum value of any sample
	 * @param _M Maximum value of any sample
	 * @param step Draw vertical axis scale using @a step increments
	 */
	CairoTSPlot(std::string text, sample_fn_t fn, double _m, double _M,
		    double step = 0)
		: __text(text), __sample_fn(fn),
		  __data(new value_struct[NUM_POINTS]), __data_i(0),
		  __data_chg(FALSE), __value_min(_m), __value_max(_M),
		__tick_step(step),__transform_matrix(Cairo::identity_matrix()) {
		get_style_context()->lookup_color("theme_text_color",
						  __text_rgba);
	}
	CairoTSPlot(const CairoTSPlot &_o) :CairoTSPlot(_o.__text,_o.__sample_fn
				, _o.__value_min,_o.__value_max, _o.__tick_step)
	{ }
	virtual ~CairoTSPlot() { }

	void set_transform_matrix(Cairo::Matrix &_m)
	{ __transform_matrix = _m;
	  queue_draw(); }

#define __circbuf_get(_b, _i) (_b[(_i) & (NUM_POINTS - 1)])
	/** Call the @a fn function (constructor argument) and rotate __data;
	 * __data[__data_i] will be the new value.
	 */
	void sample(void *arg, std::chrono::time_point<
	    std::chrono::steady_clock> _tp = std::chrono::steady_clock::now()) {
		std::chrono::duration<double> _diff = _tp - __last_tp;
		bool has_tp = (_diff.count() >= 5.0f);
		struct value_struct &_s = __circbuf_get(__data,
							__data_i.fetch_add(1));

		_s._v = __sample_fn(arg, _s._alert);
		if ((_s._has_tp = has_tp))
			__last_tp = _s._tp = _tp;
		__data_chg = TRUE;
	}
	
	void update() {
		if (__data_chg) {  // avoid invalidate_rect() if __data[]
			           // didn't change
			__data_chg = FALSE;
			get_window()->invalidate_rect
				(Gdk::Rectangle(0, 0,
						get_allocation().get_width(),
						get_allocation().get_height()),
				 FALSE);
		}
	}

protected:
	inline double yoffset_of(double value)
	{ return static_cast<int>
			((value - __value_min) / (__value_max - __value_min)
			 * __data_height); }
};

#endif /* CAIROTSPLOT_HH */
