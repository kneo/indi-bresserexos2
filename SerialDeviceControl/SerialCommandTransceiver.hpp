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
#include <iostream>
#include <vector>
#include <deque>
#include <queue>
#include <thread>

#include <algorithm>
#include "Config.hpp"
#include "INotifyPointingCoordinatesReceived.hpp"
#include "ISerialInterface.hpp"
#include "CriticalData.hpp"
#include "SerialCommand.hpp"
#include "CircularBuffer.hpp"

namespace SerialDeviceControl
{
	//These types have to inherit/implement:
	//-The ISerialInterface.hpp as Interface type.
	//-The INotifyPointingCoordinatesReceived.hpp as callback type
	template<class InterfaceType, class CallbackType>
	class SerialCommandTransceiver
	{
		public:
			
			SerialCommandTransceiver(InterfaceType& interfaceImplementation, CallbackType& dataReceivedCallback) :
				mDataReceivedCallback(dataReceivedCallback),
				mInterfaceImplementation(interfaceImplementation),
				mThreadRunning(false),
				mSerialReceiverBuffer(0x00),
				mSerialReaderThread()
			{
				SerialCommand::PushHeader(mMessageHeader);
			}
			
			virtual ~SerialCommandTransceiver()
			{
				bool threadRunning = mThreadRunning.Get();
				
				if(threadRunning)
				{
					Stop();
				}
			}
			
			virtual void Start()
			{
				mSerialReaderThread = std::thread(&SerialCommandTransceiver::SerialReaderThreadFunction,this);
			}
			

			
			void Stop()
			{
				bool running = mThreadRunning.Get();
				
				if(running)
				{
					mThreadRunning.Set(false);
				
					mSerialReaderThread.join();
				}
			}
			
		protected:
			void SendMessageBuffer(uint8_t* buffer, size_t offset, size_t length)
			{
				mInterfaceImplementation.Write(buffer,offset,length);
			}
			
		private:
		
			InterfaceType& mInterfaceImplementation;
			
			CallbackType& mDataReceivedCallback;
			
			CriticalData<bool> mThreadRunning;
						
			CircularBuffer<uint8_t,256> mSerialReceiverBuffer;
			
			std::vector<uint8_t> mMessageHeader;
			
			std::thread mSerialReaderThread; 
			
			std::vector<uint8_t> mParseBuffer;
			
			void TryParseMessagesFromBuffer()
			{
				mParseBuffer.clear();
				
				if(mSerialReceiverBuffer.Size()>0)
				{
					mSerialReceiverBuffer.CopyToVector(mParseBuffer);
					
					std::vector<uint8_t>::iterator startPosition = std::search(mParseBuffer.begin(),mParseBuffer.end(),mMessageHeader.begin(),mMessageHeader.end());
					
					std::vector<uint8_t>::iterator endPosition = startPosition + 13;
					
					if(startPosition != mParseBuffer.end() && endPosition != mParseBuffer.end())
					{
						//std::cout << "found sequence!" << std::endl;
						
						FloatByteConverter ra_bytes;
						FloatByteConverter dec_bytes;
						
						ra_bytes.bytes[0] = *(startPosition+5);
						ra_bytes.bytes[1] = *(startPosition+6);
						ra_bytes.bytes[2] = *(startPosition+7);
						ra_bytes.bytes[3] = *(startPosition+8);
						
						dec_bytes.bytes[0] = *(startPosition+9);
						dec_bytes.bytes[1] = *(startPosition+10);
						dec_bytes.bytes[2] = *(startPosition+11);
						dec_bytes.bytes[3] = *(startPosition+12);
						
						float ra = ra_bytes.decimal_number;
						float dec = dec_bytes.decimal_number;
						
						//std::cout << "RA:" << ra << " DEC:" << dec << std::endl;
						mDataReceivedCallback.OnPointingCoordinatesReceived(ra,dec);
						size_t dropCount = endPosition-mParseBuffer.begin();
						
						mSerialReceiverBuffer.DiscardFront(dropCount);
						
						//std::cout << "Receive size after :" << mSerialReceiverBuffer.Size() << " dropped " << dropCount << std::endl;
					}
				}
			}
			
			void SerialReaderThreadFunction()
			{
				//std::cout << "thread started!" << std::endl;
				bool running = mThreadRunning.Get();
				
				mInterfaceImplementation.Open();
				
				if(running==false)
				{
					mThreadRunning.Set(true);
					
					do
					{
						//controller sends status messages about every second so wait a bit and 
						std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(500));
						
						size_t bufferContent = mInterfaceImplementation.BytesToRead();
						int16_t data = -1;
						bool addSucceed = false;
						while((data = mInterfaceImplementation.ReadByte())>-1)
						{
							addSucceed = mSerialReceiverBuffer.PushBack((uint8_t)data);
						}
						
						if(addSucceed)
						{
							TryParseMessagesFromBuffer();
						}
						
						//std::cout << "Serial buffer has " << std::dec << mSerialReceiverBuffer.Size() << " bytes available" << std::endl;
						
						//Do serial business
						
						running = mThreadRunning.Get();
					}
					while(running == true);
					
					//std::cout << "thread stopped!" << std::endl;
				}
				mInterfaceImplementation.Flush();
				mInterfaceImplementation.Close();
			}
	};
}
#endif
