/* CairoGauge.cc - Gtk::DrawingArea-based analog gauge widget
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

#include "CairoGauge.hh"

void CairoGauge::draw_background(void) {
	const int width = get_allocation().get_width(),
		height = get_allocation().get_height(),
		radius = std::min(width, height) / 2;
	size_t	j = 0;
	Cairo::TextExtents _te;
	
	__background = get_window()
		->create_similar_surface(Cairo::CONTENT_COLOR_ALPHA,
					 width, height);
	auto cc = Cairo::Context::create(__background);
	
	// setup
	cc->set_antialias(Cairo::ANTIALIAS_SUBPIXEL);
	cc->translate(width / 2, height / 2);
	cc->set_line_cap(Cairo::LINE_CAP_ROUND);
	cc->set_line_width(2);
	cc->set_font_size(CAIROGAUGE_FONT_SIZE);
	Gdk::Cairo::set_source_rgba(cc, get_style_context()->
				             get_color(Gtk::STATE_FLAG_NORMAL));

	cc->arc_negative(0, 0, 0.8*radius, M_PI_4, 3*M_PI_4);
	cc->stroke();
	if (__tick_step != 0)
		for (double i = 0, _r1 = 0.78 * radius, _r2 = 0.82 * radius,
			     _r3 = 0.9 * radius;
		     i <= __value_max; i += __tick_step, j++) {
			double angle = angle_of(i);
			std::string label = std::to_string(static_cast<int>(i));
			bool is_labeled = (j % __label_step) == 0;
			double _r4 = is_labeled ? (_r1 * 0.95f) : _r1;
			
			cc->move_to(_r4 * cos(angle), _r4 * -sin(angle));
			cc->line_to(_r2 * cos(angle), _r2 * -sin(angle));
			cc->stroke();

			if (is_labeled) {
				cc->get_text_extents(label, _te);
				cc->move_to((_r3 * cos(angle)) - _te.width / 2,
					    _r3 * -sin(angle));
				cc->show_text(label);
			}
		}
	cc->get_text_extents(__text, _te);
	cc->move_to(-_te.width / 2, 0.7 * radius);
	cc->show_text(__text);
}

bool CairoGauge::on_draw(const Cairo::RefPtr<Cairo::Context> &cc) {
	const int width = get_allocation().get_width(),
		height = get_allocation().get_height(),
		radius = std::min(width, height) / 2;

	cc->set_antialias(Cairo::ANTIALIAS_SUBPIXEL);
	cc->translate(width / 2, height / 2);
	cc->transform(__transform_matrix);
	cc->set_line_cap(Cairo::LINE_CAP_ROUND);

	if (!__background)
		draw_background();
	cc->set_source(__background, -width / 2, -height / 2);
	cc->paint();

	// draw hand
	double angle = angle_of(__value);
	cc->set_line_width(3);
	cc->set_source_rgba(1, 0.2, 0.2, 1);
	cc->move_to(0, 0);
	cc->line_to(0.76*radius * cos(angle), -0.76*radius * sin(angle));
	cc->stroke();
	cc->arc(0, 0, 0.03 * radius, 0, 2*M_PI);
	cc->fill();

	return TRUE;
}
