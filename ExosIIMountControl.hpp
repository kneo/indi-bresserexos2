/*
 * ExosIIMountControl.hpp
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
 
#ifndef _EXOSIIMOUNTCONTROL_H_INCLUDED_
#define _EXOSIIMOUNTCONTROL_H_INCLUDED_

#include <cstdint>
#include <vector>
#include "Config.hpp"

#include "SerialDeviceControl/CriticalData.hpp"
#include "SerialDeviceControl/SerialCommand.hpp"
#include "SerialDeviceControl/SerialCommandTransceiver.hpp"
#include "SerialDeviceControl/INotifyPointingCoordinatesReceived.hpp"

namespace TelescopeMountControl
{
	//enum representing the telescope mount state
	enum TelescopeMountState
	{
		//intial state
		Disconnected = 0,
		//intial state, if the mount does not report the pointing coordinates.
		//if an error occurs, the telescope also will be in this state.
		Unknown = 1,
		//If the status report messages arrive and the telescope is not moving, this is the assumed state.
		//the telescope is assumed in park/initial position according to manual
		//or if the "park" command is issued.
		Parked = 2,
		//The controller autonomously desides the motion speeds, and does not report any "state".
		//This state is assumed if the motion speeds exceed a certain threshold.
		Slewing = 3,
		//This state is assumed if the telescope is moves below the slewing threshold.
		//its also the default when a "goto" is issued, since the telescope controller automatically tracks the issued coordinates.
		Tracking = 4,
		//This state is reached when issuing the stop command while slewing or tracking.
		Idle = 5,
	};
	
	//These types have to inherit/implement:
	//-The ISerialInterface.hpp as Interface type.
	template<class InterfaceType>
	class ExosIIMountControl : 
		public SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>, 
		public SerialDeviceControl::INotifyPointingCoordinatesReceived
	{
		public:
			ExosIIMountControl(InterfaceType& interfaceImplementation) : 
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>(interfaceImplementation,*this),
				mTelescopeState(TelescopeMountState::Disconnected)
			{
				//SerialDeviceControl::EquatorialCoordinates initialCoordinates;
				//mCurrentPointingCoordinats(initialCoordinates);
			}
			
			virtual ~ExosIIMountControl()
			{
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::Stop();
			}
			
			virtual void Start()
			{
				mTelescopeState.Set(TelescopeMountState::Unknown);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::Start();
			}
			virtual void Stop()
			{
				mTelescopeState.Set(TelescopeMountState::Disconnected);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::Stop();
			}
			
			void StopMotion()
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				
				if(currentState<TelescopeMountState::Unknown)
				{
					//TODO: error invalid state.
					std::cout << "Error: can not send stop, not connected." << std::endl;
					return;
				}
				
				std::vector<uint8_t> messageBuffer;
				SerialDeviceControl::SerialCommand::GetStopMotionCommandMessage(messageBuffer);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
			}
			
			void ParkPosition()
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				
				if(currentState<TelescopeMountState::Parked)
				{
					//TODO: error invalid state.
					std::cout << "Error: can not park, not connected." << std::endl;
					return;
				}
				
				std::vector<uint8_t> messageBuffer;
				SerialDeviceControl::SerialCommand::GetParkCommandMessage(messageBuffer);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
			}
			
			void GoTo(float rightAscension, float declination)
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				
				if(currentState<TelescopeMountState::Parked)
				{
					//TODO: error invalid state.
					std::cout << "Error: can not goto, invalid state." << std::endl;
					return;
				}
				
				std::vector<uint8_t> messageBuffer;
				SerialDeviceControl::SerialCommand::GetGotoCommandMessage(messageBuffer,rightAscension,declination);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
			}
			
			void SetSiteLocation(float latitude, float longitude)
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				
				if(currentState<TelescopeMountState::Parked)
				{
					//TODO: error invalid state.
					std::cout << "Error: can not set site location, invalid telescope state." << std::endl;
					return;
				}
				
				std::vector<uint8_t> messageBuffer;
				SerialDeviceControl::SerialCommand::GetSetSiteLocationCommandMessage(messageBuffer,latitude,longitude);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
			}
			
			void SetDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				std::cout << "month: " << month << std::endl;
				
				if(currentState<TelescopeMountState::Parked)
				{
					//TODO: error invalid state.
					std::cout << "Error: can not set date and time, invalid telescope state." << std::endl;
					return;
				}
				
				std::vector<uint8_t> messageBuffer;
				SerialDeviceControl::SerialCommand::GetSetDateTimeCommandMessage(messageBuffer,year,month,day,hour,minute,second);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
			}

			//Called each time a pair of coordinates was received from the serial interface.
			virtual void OnPointingCoordinatesReceived(float right_ascension, float declination)
			{
				//std::cout << "Received data : RA: " << right_ascension << " DEC:" << declination << std::endl;
				
				SerialDeviceControl::EquatorialCoordinates lastCoordinates = mCurrentPointingCoordinates.Get();
				
				SerialDeviceControl::EquatorialCoordinates coordinatesReceived;
				coordinatesReceived.RightAscension = right_ascension;
				coordinatesReceived.Declination = declination;
				
				mCurrentPointingCoordinates.Set(coordinatesReceived);
				
				SerialDeviceControl::EquatorialCoordinates delta = SerialDeviceControl::EquatorialCoordinates::Delta(lastCoordinates,coordinatesReceived);
				float absDelta = SerialDeviceControl::EquatorialCoordinates::Absolute(delta);
				
				TelescopeMountState currentState = mTelescopeState.Get();
				
				switch(currentState)
				{
					case TelescopeMountState::Unknown:
						//change to the parked state, resolving the unknown state.
						mTelescopeState.Set(TelescopeMountState::Parked);
					break;
					
					case TelescopeMountState::Parked:
					case TelescopeMountState::Idle:
						//std::cout << "Delta : RA: " << delta.RightAscension << " DEC:" << delta.Declination << " abs :" << absDelta << std::endl;
					break;
					
					default:
						
					break;
				}
			}
			
			TelescopeMountState GetTelescopeState()
			{
				return mTelescopeState.Get();
			}
			
			SerialDeviceControl::EquatorialCoordinates GetPointingCoordinates()
			{
				return mCurrentPointingCoordinates.Get();
			}
			
		private:
			SerialDeviceControl::CriticalData<SerialDeviceControl::EquatorialCoordinates> mCurrentPointingCoordinates;
			SerialDeviceControl::CriticalData<TelescopeMountState> mTelescopeState;
	};
}
#endif
