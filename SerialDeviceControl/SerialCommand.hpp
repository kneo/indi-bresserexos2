/*
 * SerialCommand.hpp
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
 
#ifndef _SERIALCOMMAND_H_INCLUDED_
#define _SERIALCOMMAND_H_INCLUDED_

#include <cstdint>
#include <vector>
#include "Config.hpp"

namespace SerialDeviceControl
{
	//After determining the message frame size and structure, 
	//these commands where found to affect the telescope controller (Handbox + Motors).
	//If an invalid command is issued the telescope controller stops reporting its status,
	//until another valid command is issued.
	//Command IDs which did not stop the report messages are considered effective and are listed below: 
	enum SerialCommandID
	{
		//these command IDs do not stop the controller report, 
		//but do not have an appearent effect either.
		UNKNOWN_COMMAND_ID_1 = (uint8_t)0x01,
		UNKNOWN_COMMAND_ID_2 = (uint8_t)0x02,
		UNKNOWN_COMMAND_ID_4 = (uint8_t)0x04,
		UNKNOWN_COMMAND_ID_8 = (uint8_t)0x08,
		
		//immediatly stops slewing the telescope.
		STOP_MOTION_COMMAND_ID = (uint8_t)0x1D,
		
		//slews the telescope back into the park/initial position.
		PARK_COMMAND_ID = (uint8_t)0x1E,
		
		//Slews the telescope to the equatorial coordinates provided.
		GOTO_COMMAND_ID = (uint8_t)0x23,
		
		//sets the site location on the telescope controller.
		SET_SITE_LOCATION_COMMAND_ID = (uint8_t)0x25,
		
		//sets time and date on the telescope controller.
		SET_DATE_TIME_COMMAND_ID = (uint8_t)0x26,
		
		//This id is used by the telescope controller to 
		TELESCOPE_POSITION_REPORT_COMMAND_ID = (uint8_t)0xff
	};
	
	//Enum with month names for easy legibility.
	enum DateMonths
	{
		January = 1,
		February = 2,
		March = 3,
		April = 4,
		May = 5,
		June = 6,
		July = 7,
		August = 8,
		September = 9,
		October = 10,
		November = 11,
		December = 12,
	};
	
	//helper union to read out float bytes without the hazzle with pointers
	union FloatByteConverter
	{
		uint8_t bytes[4];
		float decimal_number;
	};
	
	//Simple static class providing the message generation mechanisms.
	//The message frame size is 13 bytes, a 4 byte header/preamble, 
	//a one byte command followed by 2 to 6 arguments, 
	//distributed over the 8 remaining bytes.
	//see the Get...Message() implementations for details.
	//Since the serial protocol is faily simple, a lot of the error handling is on the client side to avoid the controller to go haywire.
	class SerialCommand
	{
		private:

			
			//helper function pushing a number of bytes into the buffer, for padding.
			static void push_bytes(std::vector<uint8_t>& buffer,uint8_t byte, size_t count);
			
			//simple constant containing the message header as of firmware V2.3.
			static uint8_t MessageHeader[4];
			
			//helper function to push the float values into the buffer
			static void push_float_bytes(std::vector<uint8_t>& buffer,FloatByteConverter& values);
		
		public:
			//put the stop message into the buffer provided.
			//returns false if an error occurs.
			static bool GetStopMotionCommandMessage(std::vector<uint8_t>& buffer);
			
			//put the park message into the buffer provided.
			//returns false if an error occurs.
			static bool GetParkCommandMessage(std::vector<uint8_t>& buffer);
			
			//put the goto message corresponding to the coordinates provided into the buffer provided.
			//returns false if an error occurs.
			static bool GetGotoCommandMessage(std::vector<uint8_t>& buffer,float decimal_right_ascension, float decimal_declination);
			
			//put the set site location message corresponding the coordinates provided into the buffer provided
			//returns false if an error occurs.
			static bool GetSetSiteLocationCommandMessage(std::vector<uint8_t>& buffer, float decimal_latitude, float decimal_longitude);
			
			//put the date time message corresponding to the time/date provided into the buffer provided.
			//returns false if an error occurs.
			static bool GetSetDateTimeCommandMessage(std::vector<uint8_t>& buffer, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
			
			//helper function pushing the header into the buffer.
			static void PushHeader(std::vector<uint8_t>& buffer);
			

	};
}

#endif
