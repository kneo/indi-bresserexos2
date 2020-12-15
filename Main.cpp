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
#include <string>
#include <chrono>
#include <thread>



#include "Config.hpp"
#include "SerialDeviceControl/SerialCommand.hpp"
#include "TestSerialImplementation.hpp"
#include "TestDataReceivedCallback.hpp"
#include "SerialCommandTransceiver.hpp"
#include "ExosIIMountControl.hpp"

using namespace SerialDeviceControl;
using namespace Testing;

void dump_message(std::vector<uint8_t>& buffer)
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

void dump_buffer(std::vector<uint8_t>& buffer)
{
	if(buffer.size() == 0)
	{
		return;
	}
	
	std::cout << "Size is : " << std::dec << buffer.size() << std::endl;
		
	for(int i = 0;i<buffer.size(); i++)
	{
		std::cout << std::hex << static_cast<int>(buffer[i]) << " ";
	}
	std::cout << std::dec << "\n" << std::endl;
}

int main(int argc, char **argv)
{
	if(argc<2)
	{
		std::cout << "missing serial device!" << std::endl;
		std::cout << "Usage: " << argv[0] << " /dev/path/to/ttyDev" << std::endl;
		return -1;
	}
	
	//DriverTest
	std::cout << argv[0] << " Version " << BresserExosIIGoToDriverForIndi_VERSION_MAJOR << "." << BresserExosIIGoToDriverForIndi_VERSION_MINOR << "\n" << std::endl;
	std::cout << "using device:" << argv[1] << std::endl;
	std::vector<uint8_t> message;
	
	/*if(SerialDeviceControl::SerialCommand::GetStopMotionCommandMessage(message))
	{
		dump_message(message);
		message.clear();
	}
	
	if(SerialDeviceControl::SerialCommand::GetParkCommandMessage(message))
	{
		dump_message(message);
		message.clear();
	}

	if(SerialDeviceControl::SerialCommand::GetGotoCommandMessage(message, 6.0,90.0))
	{
		dump_message(message);
		message.clear();
	}
	
	if(SerialDeviceControl::SerialCommand::GetSetSiteLocationCommandMessage(message, 52.0,13.0))
	{
		dump_message(message);
		message.clear();
	}	
	
	if(SerialDeviceControl::SerialCommand::GetSetDateTimeCommandMessage(message,2020,12,12,12,12,00))
	{
		dump_message(message);
		message.clear();
	}*/
	
	//std::vector<uint8_t> readBuffer;
	
	SerialDeviceControl::SerialCommand::GetParkCommandMessage(message);
	
	std::string portName(argv[1]);
	
	TestSerialImplementation implementation(portName,B9600);
	TestDataReceivedCallback cb;
	TelescopeMountControl::ExosIIMountControl<TestSerialImplementation> mountControl(implementation);
	
	bool running = true;
	
	do
	{
		int menuPoint = -1;
		std::string input;
				
		std::cout << "[1] connect to telescope" << std::endl;
		std::cout << "[2] disconnect telescope" << std::endl;
		std::cout << "[3] show current pointing coordinates" << std::endl;
		std::cout << "[4] Set Location" << std::endl;
		std::cout << "[5] Set Date and Time" << std::endl;
		std::cout << "[6] Stop Motion" << std::endl;
		std::cout << "[7] Park Telescope" << std::endl;
		std::cout << "[8] GoTo and Track" << std::endl;
		
		std::cout << std::endl;
		std::cout << "[0] Quit Program" << std::endl;
		
		std::cout << "Please select:>";
		std::getline(std::cin,input);
		menuPoint = std::stoi(input);
		
		SerialDeviceControl::EquatorialCoordinates coords = mountControl.GetPointingCoordinates();
		

		
		float lat;
		float lon;
		
		float ra;
		float dec;
		
		uint16_t year;
		uint8_t month;
		uint8_t day;

		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		
		switch(menuPoint)
		{
			case 0:
				running = false;
			break;
			
			case 1:
				if(mountControl.GetTelescopeState() == TelescopeMountControl::TelescopeMountState::Disconnected)
				{
					mountControl.Start();
				}
			break;
			
			case 2:
				if(mountControl.GetTelescopeState() != TelescopeMountControl::TelescopeMountState::Disconnected)
				{
					mountControl.Stop();
				}
			break;
			
			case 3:
				std::cout << "Currently Pointing to RA: " << coords.RightAscension << " DEC:" << coords.Declination << std::endl;
			break;
			
			case 4:
				std::cout << "Enter decimal location latitude:>";
				std::getline(std::cin,input);
				lat = std::stof(input);
				
				std::cout << "Enter decimal location longitude:>";
				std::getline(std::cin,input);
				lon = std::stof(input);
				
				mountControl.SetSiteLocation(lat,lon);
			break;
			
			case 5:
				std::cout << "Enter year 0-9999:>";
				std::getline(std::cin,input);
				year = std::stoi(input);
				
				std::cout << "Enter month:>";
				std::getline(std::cin,input);
				month = std::stoi(input);
				
				std::cout << "Enter day:>";
				std::getline(std::cin,input);
				day = std::stoi(input);
				
				std::cout << "Enter hour 0-23:>";
				std::getline(std::cin,input);
				hour = std::stoi(input);
				
				std::cout << "Enter minute 0-59:>";
				std::getline(std::cin,input);
				minute = std::stoi(input);
				
				std::cout << "Enter second 0-59:>";
				std::getline(std::cin,input);
				second = std::stoi(input);
				
				mountControl.SetDateTime(year,month,day,hour,minute,second);
				
			break;
			
			case 6:
				mountControl.StopMotion();
			break;
			
			case 7:
				mountControl.ParkPosition();
			break;
			
			case 8:
				std::cout << "Enter decimal right ascension:>";
				std::getline(std::cin,input);
				ra = std::stof(input);
				
				std::cout << "Enter decimal declination:>";
				std::getline(std::cin,input);
				dec = std::stof(input);
				
				mountControl.GoTo(ra,dec);
			break;
			
			default:
				std::cout<<"invalid selection, reconsider!"<<std::endl;
			break;
		}
		
	}while(running);
	
	//mountControl.Start();
	
	//use this to start the telescope position reporting.
	//implementation.Open();
	//implementation.Write(&message[0],0,message.size());
	
	//while(true);
	
	//std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::seconds(10));
	
	//mountControl.Stop();
	
	//std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::seconds(3));
	
	return 0;
}

