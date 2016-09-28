/* CairoTSPlot.cc - a time-series plot widget
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

#include "CairoTSPlot.hh"

void CairoTSPlot::draw_background(void) {
	const int width = get_allocation().get_width(),
		height = get_allocation().get_height(),
		_y0 = height - MARGIN_BOTTOM;
	Cairo::TextExtents _te;
	
	__background = get_window()
		->create_similar_surface(Cairo::CONTENT_COLOR_ALPHA,
					 width, height);
	auto cc = Cairo::Context::create(__background);
	
	cc->set_antialias(Cairo::ANTIALIAS_SUBPIXEL);
	cc->set_line_cap(Cairo::LINE_CAP_ROUND);
	cc->set_line_width(1);

	// background
	cc->set_source_rgba(1, 1, 1, 1);
	cc->rectangle(MARGIN_LEFT, MARGIN_TOP,
		      width - MARGIN_LEFT - MARGIN_RIGHT,
		      height - MARGIN_TOP - MARGIN_BOTTOM);
	cc->fill();
	cc->translate(-0.5f, -0.5f);  // avoid AA blur

	// vertical axis scale and borders
	if (__tick_step != 0)
		for (double i = __value_min; i <= __value_max;i +=__tick_step) {
			double _y = yoffset_of(i);
			std::string label = std::to_string(static_cast<int>(i));

			if (i == __value_min || i == __value_max)
				cc->set_source_rgba(0.70, 0.71, 0.70, 1);
			else
				cc->set_source_rgba(0.89, 0.89, 0.89, 1);
			cc->move_to(MARGIN_LEFT, _y0 - _y);
			cc->line_to(width - MARGIN_RIGHT + 4, _y0 - _y);
			cc->stroke();

			Gdk::Cairo::set_source_rgba(cc, __text_rgba);
			cc->get_text_extents(label, _te);
			cc->move_to(width - MARGIN_RIGHT + 6,
				    _y0 - _y + (_te.height / 2));
			cc->show_text(label);
		}
	cc->set_source_rgba(0.70, 0.71, 0.70, 1);
	cc->move_to(MARGIN_LEFT, MARGIN_TOP);
	cc->line_to(MARGIN_LEFT, height - MARGIN_BOTTOM);
	cc->move_to(width - MARGIN_RIGHT, MARGIN_TOP);
	cc->line_to(width - MARGIN_RIGHT, height - MARGIN_BOTTOM);
	cc->stroke();

	// draw the __text string
	Gdk::Cairo::set_source_rgba(cc, __text_rgba);
	cc->set_font_size(CAIROTSPLOT_FONT_SIZE);
	cc->get_text_extents(__text, _te);
	cc->move_to((width - _te.width) / 2, MARGIN_TOP / 2);
	cc->show_text(__text);
}

bool CairoTSPlot::on_draw(const Cairo::RefPtr<Cairo::Context> &cc) {
	const int width = get_allocation().get_width(),
		height = get_allocation().get_height(),
		_y0 = (height / 2) - MARGIN_BOTTOM,
		_x_offset = (width / 2) - MARGIN_RIGHT;
	const double _xstep = (width - MARGIN_LEFT
			       - MARGIN_RIGHT)/ static_cast<double>(NUM_POINTS);
	ssize_t _i = __data_i.load() - 1, _last;
	Cairo::TextExtents _te;
	
	cc->set_antialias(Cairo::ANTIALIAS_SUBPIXEL);
	cc->translate((width / 2) - 0.5f, (height / 2) - 0.5f);
	cc->transform(__transform_matrix);
	cc->set_line_cap(Cairo::LINE_CAP_ROUND);
	cc->set_line_join(Cairo::LINE_JOIN_ROUND);
	
	if (!__background)
		draw_background();
	cc->set_source(__background, -(width / 2) - 0.5f,  // avoid AA blur
		       -(height / 2) - 0.5f);
	cc->paint();
	cc->set_line_width(1);
	
	// horizontal axis scale
	auto _tp = std::chrono::steady_clock::now();
	for (int i = 0; i < NUM_POINTS; ++i) {
		struct value_struct &_s = __circbuf_get(__data, _i - i);
		if (_s._has_tp) {
			std::chrono::duration<double> diff = _tp - _s._tp;
			std::string label = std::to_string(static_cast<int>
							  (diff.count())) + "s";

			cc->set_source_rgba(0.89, 0.89, 0.89, 1);
			cc->move_to(_x_offset - (_xstep * i),
				    _y0 - __data_height);
			cc->line_to(_x_offset - (_xstep * i),
				    _y0 + 4);
			cc->stroke();

			Gdk::Cairo::set_source_rgba(cc, __text_rgba);
			cc->get_text_extents(label, _te);
			cc->move_to(_x_offset - (_xstep * i) - (_te.width / 2),
				    _y0 + 11);
			cc->show_text(label);
		}
	}

	// draw plot
	Gdk::Cairo::set_source_rgba(cc, (_last = __circbuf_get(__data, _i)
				      ._alert) ? __RGBA_ALERT : __RGBA_DEFAULT);
	cc->set_line_width(2);
	cc->move_to(_x_offset, _y0 - yoffset_of(__circbuf_get(__data, _i)._v));
	for (int i = 1; i < NUM_POINTS; ++i) {
		struct value_struct &_s = __circbuf_get(__data, _i - i);
		if (_s._v == HUGE_VAL)
			break;

		cc->line_to(_x_offset - (_xstep * i),
			    _y0 - yoffset_of(_s._v));
		if (_last ^ _s._alert) { /* set a different RGBA and
					  * continue path if _s._alert
					  * changed */
			cc->stroke();
			Gdk::Cairo::set_source_rgba(cc, (_last =
				  __circbuf_get(__data, _i - i)._alert)
				       ? __RGBA_ALERT : __RGBA_DEFAULT);
			cc->move_to(_x_offset - (_xstep * i),
				    _y0 - yoffset_of(_s._v));
		}
	}
	cc->stroke();
	return TRUE;
}
