/*
 * Main.cpp
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

#include <iostream>
#include <vector>
#include "Config.hpp"
#include "SerialDeviceControl/SerialCommand.hpp"

using SerialDeviceControl::SerialCommand;

void dump_buffer(std::vector<uint8_t>& buffer)
{
		if(buffer.size() != 13)
		{
			std::cout << "Size mismatch of message buffer: " << std::dec << buffer.size() << std::endl;
		}
		
		switch(buffer[4])
		{
			case SerialDeviceControl::SerialCommandID::STOP_MOTION_COMMAND_ID:
			std::cout << "Message is STOP_MOTION_COMMAND_ID" << std::endl;
			break;

			case SerialDeviceControl::SerialCommandID::PARK_COMMAND_ID:
			std::cout << "Message is PARK_COMMAND_ID" << std::endl;
			break;
			
			case SerialDeviceControl::SerialCommandID::GOTO_COMMAND_ID:
			std::cout << "Message is GOTO_COMMAND_ID" << std::endl;
			break;
			
			case SerialDeviceControl::SerialCommandID::SET_SITE_LOCATION_COMMAND_ID:
			std::cout << "Message is SET_SITE_LOCATION_COMMAND_ID" << std::endl;
			break;
			
			case SerialDeviceControl::SerialCommandID::SET_DATE_TIME_COMMAND_ID:
			std::cout << "Message is SET_DATE_TIME_COMMAND_ID" << std::endl;
			break;
			
			default:
			std::cout << "Message is UNKNOWN_ID" << buffer[4] << std::endl;
			break;
		}
		
		std::cout << "Size is : " << std::dec << buffer.size() << std::endl;
		
		for(int i = 0;i<buffer.size(); i++)
		{
			std::cout << std::hex << static_cast<int>(buffer[i]) << " ";
		}
		std::cout << "\n" << std::endl;
}

int main(int argc, char **argv)
{
	//DriverTest
	std::cout << argv[0] << " Version " << BresserExosIIGoToDriverForIndi_VERSION_MAJOR << "." << BresserExosIIGoToDriverForIndi_VERSION_MINOR << "\n" << std::endl;
	
	std::vector<uint8_t> message;
	
	if(SerialDeviceControl::SerialCommand::GetStopMotionCommandMessage(message))
	{
		dump_buffer(message);
		message.clear();
	}
	
	if(SerialDeviceControl::SerialCommand::GetParkCommandMessage(message))
	{
		dump_buffer(message);
		message.clear();
	}

	if(SerialDeviceControl::SerialCommand::GetGotoCommandMessage(message, 6.0,90.0))
	{
		dump_buffer(message);
		message.clear();
	}
	
	if(SerialDeviceControl::SerialCommand::GetSetSiteLocationCommandMessage(message, 52.0,13.0))
	{
		dump_buffer(message);
		message.clear();
	}	
	
	if(SerialDeviceControl::SerialCommand::GetSetDateTimeCommandMessage(message,2020,12,12,12,12,00))
	{
		dump_buffer(message);
		message.clear();
	}
	
	return 0;
}

