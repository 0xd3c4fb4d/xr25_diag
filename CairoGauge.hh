/* CairoGauge.hh - Gtk::DrawingArea-based analog gauge widget
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

#ifndef CAIROGAUGE_HH
#define CAIROGAUGE_HH

#include <string>
#include <functional>
#include <cmath>
#include <gtkmm.h>
#include <cairomm/context.h>

#define CAIROGAUGE_FONT_SIZE 14

class CairoGauge : public Gtk::DrawingArea {
protected:
	typedef std::function<double(void *)> sample_fn_t;

	std::string  __text;
	sample_fn_t  __sample_fn;
	double       __value, __value_max,
		__tick_step;
	size_t       __label_step;
	Cairo::Matrix __transform_matrix;
	Cairo::RefPtr<Cairo::Surface> __background;

	void draw_background(void);
	
	// Override Gtk::DrawingArea::on_draw() signal handler
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cc) override;
	// Override Gtk::Widget::on_size_allocate() signal handler
	void on_size_allocate(Gtk::Allocation& allocation) override {
		Gtk::Widget::on_size_allocate(allocation);
		if (__background)
			draw_background();
	}
public:
	/** Construct a CairoGauge object
	 * @param text Text rendered below the gauge
	 * @param fn std::function<double(void *)> that returns the next value
	 * @param _M Maximum value of any sample
	 * @param step Draw ticks using @a step increments
	 * @param l_step Draw labels each @a l_step ticks
	 */
	CairoGauge(std::string text, sample_fn_t fn, double _M, double step = 0,
		   size_t l_step = 1)
		: __text(text), __sample_fn(fn), __value(0), __value_max(_M),
		  __tick_step(step), __label_step(l_step),
		  __transform_matrix(Cairo::identity_matrix()) { }
	CairoGauge(const CairoGauge &_o) : CairoGauge(_o.__text, _o.__sample_fn,
						_o.__value_max, _o.__tick_step)
	{ }
	virtual ~CairoGauge() { }

	void set_transform_matrix(Cairo::Matrix &_m)
	{ __transform_matrix = _m;
	  queue_draw(); }
	
	/** Call the @a fn function (constructor argument) and update gauge with
	 * the returned value.
	 */
	void update(void *arg) {
		auto v = __sample_fn(arg);
		if (v != __value) {  // avoid invalidate_rect() if the value
			             // didn't change
			__value = v;
			get_window()->invalidate_rect
				(Gdk::Rectangle(0, 0,
						get_allocation().get_width(),
						get_allocation().get_height()),
				 FALSE);
		}
	}

protected:
	inline double angle_of(double value)
	{ return 5*M_PI_4 - (value / __value_max * 3*M_PI_2); }
};

#endif /* CAIROGAUGE_HH */
