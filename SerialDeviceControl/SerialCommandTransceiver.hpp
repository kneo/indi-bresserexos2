/*
 * SerialCommandTransceiver.hpp
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
 
#ifndef _SERIALCOMMANDTRANSCEIVER_H_INCLUDED_
#define _SERIALCOMMANDTRANSCEIVER_H_INCLUDED_

#include <cstdint>
#include <vector>
#include <thread>
#include <chrono>

#include "Config.hpp"

#include "INotifyPointingCoordinatesReceived.hpp"
#include "ISerialInterface.hpp"
#include "CriticalData.hpp"

namespace SerialDeviceControl
{
	//Simple data structure for a coordinate pair.
	struct EquatorialCoordinates
	{
		//The time stamp when this coordinates where received.
		std::chrono::time_point<std::chrono::system_clock> TimeStamp;
		
		//decimal value of the right ascension.
		float RightAscension;
		
		//decimal value of the declination.
		float Declination;
	};
	
	class SerialCommandTransceiver
	{
		public:
			
			SerialCommandTransceiver(ISerialInterface* interfaceImplementation, INotifyPointingCoordinatesReceived* dataReceivedCallback);
			
			virtual ~SerialCommandTransceiver();
			
			void Start();
			
			void SendMessageBuffer(uint8_t* buffer, size_t offset, size_t length);
			
			void Stop();
			
		private:
		
			ISerialInterface* mInterfaceImplementation;
			
			INotifyPointingCoordinatesReceived* mDataReceivedCallback;
			
			CriticalData<bool> mThreadRunning;
			
			std::vector<EquatorialCoordinates> mReceivedMessages;
			
			std::vector<uint8_t> mSerialReceiverBuffer;
			
			void TryParseMessagesFromBuffer();
			
			void SerialReaderThreadFunction();
	};
}
#endif
