/* UI.cc  - xr25_diag user interface (gtkmm)
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

#include <iomanip>
#include <sstream>
#include "UI.hh"

#define update_entry(_i, _c) __entry[_i]->set_text(std::to_string(_c))
#define update_flag(_i, _c)  __flag[_i]->set((_c) ? Gtk::ARROW_RIGHT \
				      : Gtk::ARROW_NONE, Gtk::SHADOW_OUT)
void UI::update_page_diagnostic(XR25frame &fra) {
	update_entry(E_PROGRAM_VRSN,    fra.program_vrsn);
	update_entry(E_CALIB_VRSN,      fra.calib_vrsn);
	update_entry(E_MAP,             fra.map);
	update_entry(E_RPM,             fra.rpm);
	update_entry(E_THROTTLE,        fra.throttle);
	update_entry(E_ENG_PINGING,     fra.eng_pinging);
	update_entry(E_INJECTION_US,    fra.injection_us);
	update_entry(E_ADVANCE,         fra.advance);
	update_entry(E_TEMP_WATER,      fra.temp_water);
	update_entry(E_TEMP_AIR,        fra.temp_air);
	update_entry(E_BATT_V,          fra.batt_v);
	update_entry(E_LAMBDA_V,        fra.lambda_v);
	update_entry(E_IDLE_REGULATION, fra.idle_regulation);
	update_entry(E_IDLE_PERIOD,     fra.idle_period);
	update_entry(E_ENG_PINGING_DELAY, fra.eng_pinging_delay);
	update_entry(E_ATMOS_PRESSURE,  fra.atmos_pressure);
	update_entry(E_AFR_CORRECTION,  fra.afr_correction);
	update_entry(E_SPD_KM_H,        fra.spd_km_h);

	update_flag(F_IN_AC_REQUEST,       fra.in_flags & IN_AC_REQUEST);
	update_flag(F_IN_AC_COMPRES,       fra.in_flags & IN_AC_COMPRES);
	update_flag(F_IN_THROTTLE_0,       fra.in_flags & IN_THROTTLE_0);
	update_flag(F_IN_PARKED,           ~fra.in_flags & IN_NOT_PARKED);
	update_flag(F_IN_THROTTLE_1,       fra.in_flags & IN_THROTTLE_1);
	update_flag(F_OUT_PUMP_ENABLE,     fra.out_flags & OUT_PUMP_ENABLE);
	update_flag(F_OUT_IDLE_REGULATION, fra.out_flags & OUT_IDLE_REGULATION);
	update_flag(F_OUT_WASTEGATE_REG,   fra.out_flags & OUT_WASTEGATE_REG);
	update_flag(F_OUT_EGR_ENABLE,      fra.out_flags & OUT_EGR_ENABLE);
	update_flag(F_OUT_CHECK_ENGINE,    fra.out_flags & OUT_CHECK_ENGINE);
	update_flag(F_FAULT_MAP,           fra.fault_flags_1 & FAULT_MAP);
	update_flag(F_FAULT_SPD_SENSOR,
		    fra.fault_flags_1 & FAULT_SPD_SENSOR);
	update_flag(F_FAULT_LAMBDA_TMP,
		    fra.fault_flags_1 & FAULT_LAMBDA_TMP);
	update_flag(F_FAULT_LAMBDA,        fra.fault_flags_1 & FAULT_LAMBDA);
	update_flag(F_FAULT_WATER_OPEN_C,
		    fra.fault_flags_0 & FAULT_WATER_OPEN_C);
	update_flag(F_FAULT_WATER_SHORT_C,
		    fra.fault_flags_0 & FAULT_WATER_SHORT_C);
	update_flag(F_FAULT_AIR_OPEN_C,
		    fra.fault_flags_0 & FAULT_AIR_OPEN_C);
	update_flag(F_FAULT_AIR_SHORT_C,
		    fra.fault_flags_0 & FAULT_AIR_SHORT_C);
	update_flag(F_FAULT_TPS_LOW,       fra.fault_flags_0 & FAULT_TPS_LOW);
	update_flag(F_FAULT_TPS_HIGH,      fra.fault_flags_0 & FAULT_TPS_HIGH);
	update_flag(F_FAULT_F_WATER_OPEN_C,
		    fra.fault_fugitive & FAULT_WATER_OPEN_C);
	update_flag(F_FAULT_F_WATER_SHORT_C,
		    fra.fault_fugitive & FAULT_WATER_SHORT_C);
	update_flag(F_FAULT_F_AIR_OPEN_C,
		    fra.fault_fugitive & FAULT_AIR_OPEN_C);
	update_flag(F_FAULT_F_AIR_SHORT_C,
		    fra.fault_fugitive & FAULT_AIR_SHORT_C);
	update_flag(F_FAULT_F_TPS_LOW,     fra.fault_fugitive & FAULT_TPS_LOW);
	update_flag(F_FAULT_F_TPS_HIGH,    fra.fault_fugitive & FAULT_TPS_HIGH);
	update_flag(F_FAULT_EEPROM_CHECKSUM,
		    fra.fault_flags_2 & FAULT_EEPROM_CHECKSUM);
	update_flag(F_FAULT_PROG_CHECKSUM,
		    fra.fault_flags_2 & FAULT_PROG_CHECKSUM);
	update_flag(F_FAULT_PUMP,          ~fra.fault_flags_4 & FAULT_NOT_PUMP);
	update_flag(F_FAULT_WASTEGATE,
		    ~fra.fault_flags_4 & FAULT_NOT_WASTEGATE);
	update_flag(F_FAULT_EGR,           ~fra.fault_flags_4 & FAULT_NOT_EGR);
	update_flag(F_FAULT_IDLE_REG,
		    ~fra.fault_flags_4 & FAULT_NOT_IDLE_REG);
	update_flag(F_FAULT_INJECTORS,     fra.fault_flags_3 & FAULT_INJECTORS);
}

void UI::update_page_dashboard(XR25frame &fra)  {
	for (auto &i : __gauge)
		i.update(&fra);

	std::ostringstream oss;
	oss << std::setw(4)
	    << "MAP   (mbar) = " << fra.map      << std::endl
	    << "Air temp (C) = " << fra.temp_air << std::endl
	    << "Lambda  (mV) = " << fra.lambda_v;
	__dash_tb->set_text(oss.str());
}
