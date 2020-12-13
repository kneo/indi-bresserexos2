/*
 * SerialCommand.cpp
 * 
 * Copyright 2020 Kevin Krüger <kkevin@gmx.net>
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
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include "SerialCommand.hpp"

#ifdef USE_CERR_LOGGING
#include <iostream>
#endif

#define ERROR_NULL_BUFFER ("error: buffer is null pointer.")
#define ERROR_INVALID_RA_RANGE ("error: invalid range for right ascension.")
#define ERROR_INVALID_DEC_RANGE ("error: invalid range for declination.")
#define ERROR_INVALID_LAT_RANGE ("error: invalid range for latitude.")
#define ERROR_INVALID_LON_RANGE ("error: invalid range for longitude.")
#define ERROR_INVALID_YEAR_RANGE ("error: invalid range for year.")
#define ERROR_INVALID_MONTH_RANGE ("error: invalid range for month.")
#define ERROR_INVALID_DAY_RANGE ("error: invalid range for day.")
#define ERROR_INVALID_HOUR_RANGE ("error: invalid range for hour.")
#define ERROR_INVALID_MINUTE_RANGE ("error: invalid range for minute.")
#define ERROR_INVALID_SECOND_RANGE ("error: invalid range for second.")

uint8_t SerialDeviceControl::SerialCommand::smMessageHeader[4] = {0x55, 0xaa, 0x01, 0x09};

bool SerialDeviceControl::SerialCommand::GetStopMotionCommandMessage(uint8_t* buffer)
{
	if(buffer==nullptr)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_NULL_BUFFER << std::endl;
#endif
		return false;
	}
	
	return false;
}
			
bool SerialDeviceControl::SerialCommand::GetParkCommandMessage(uint8_t* buffer)
{
	if(buffer==nullptr)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_NULL_BUFFER << std::endl;
#endif
		return false;
	}
	
	return false;
}
			
bool SerialDeviceControl::SerialCommand::GetGotoCommandMessage(uint8_t* buffer,float decimal_right_ascension, float decimal_declination)
{
	if(decimal_right_ascension<0 && decimal_right_ascension>24)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_INVALID_RA_RANGE << std::endl;
#endif
		return false;
	}
	
	if(decimal_declination < -90 && decimal_declination > 90)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_INVALID_DEC_RANGE << std::endl;
#endif		
		return false;
	}
	
	if(buffer==nullptr)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_NULL_BUFFER << std::endl;
#endif
		return false;
	}
	
	return false;
}
			
bool SerialDeviceControl::SerialCommand::GetSetSiteLocationCommandMessage(uint8_t* buffer, float decimal_latitude, float decimal_longitude)
{
	if(decimal_latitude < -180 && decimal_latitude > 180)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_INVALID_LAT_RANGE << std::endl;
#endif
		return false;
	}

	if(decimal_longitude < -90 && decimal_longitude > 90)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_INVALID_LON_RANGE << std::endl;
#endif
		return false;
	}
	
	if(buffer==nullptr)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_NULL_BUFFER << std::endl;
#endif
		return false;
	}
	
	return false;
}
			
bool SerialDeviceControl::SerialCommand::GetSetDateTimeCommandMessage(uint8_t* buffer, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
	if(year > 9999)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_INVALID_YEAR_RANGE << std::endl;
#endif
		return false;
	}
	
	if(month<1 && month>12)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_INVALID_MONTH_RANGE << std::endl;
#endif
		return false;
	}
	
	if(day < 1 && day > 31)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_INVALID_DAY_RANGE << std::endl;
#endif
		return false;
	}
	
	if(hour > 24)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_INVALID_HOUR_RANGE << std::endl;
#endif
		return false;
	}
	
	if(minute > 59)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_INVALID_MINUTE_RANGE << std::endl;
#endif
		return false;
	}
	
	if(second > 59)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_INVALID_SECOND_RANGE << std::endl;
#endif
		return false;
	}
	
	if(buffer==nullptr)
	{
#ifdef USE_CERR_LOGGING
		std::cerr << ERROR_NULL_BUFFER << std::endl;
#endif
		return false;
	}
	
	return false;
}
