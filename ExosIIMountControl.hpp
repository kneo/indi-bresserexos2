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
#include <limits>
#include "Config.hpp"

#include "SerialDeviceControl/CriticalData.hpp"
#include "SerialDeviceControl/SerialCommand.hpp"
#include "SerialDeviceControl/SerialCommandTransceiver.hpp"
#include "SerialDeviceControl/INotifyPointingCoordinatesReceived.hpp"

//The manual states a tracking speed for 0.004°/s everything above is considered slewing.
#define TRACK_SLEW_THRESHOLD (0.005)

namespace TelescopeMountControl
{
	//enum representing the telescope mount state
	enum TelescopeMountState
	{
		//intial state, no serial connection established
		Disconnected = 0,
		//intial state if the serial connection was established,
		//the mount did not report any pointing coordinates yet.
		//if an error occurs, the telescope also will be in this state.
		Unknown = 1,
		//If the user issues the park command the telescope likely needs to slew to the parking position, 
		//this is determined by the differentials of the positions information send by the controller.
		ParkingIssued = 2,
		//if parking was issued and the telescope is found moving this state is active.
		SlewingToParkingPosition = 3,
		//If the status report messages arrive and the telescope is not moving, this is the assumed state.
		//the telescope is assumed in park/initial position according to manual
		//or if the "park" command is issued.
		Parked = 4,
		//The controller autonomously desides the motion speeds, and does not report any "state".
		//This state is assumed if the motion speeds exceed a certain threshold.
		Slewing = 5,
		//The user ordered the telescope to track/goto a specific location. State of motion has to be determined by the report message.
		TrackingIssued = 6,
		//This state is assumed if the telescope is moves below the slewing threshold.
		//its also the default when a "goto" is issued, since the telescope controller automatically tracks the issued coordinates.
		Tracking = 7,
		//This state is reached when issuing the stop command while slewing or tracking.
		Idle = 8,
	};
	
	//These types have to inherit/implement:
	//-The ISerialInterface.hpp as Interface type.
	template<class InterfaceType>
	class ExosIIMountControl : 
		public SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>, 
		public SerialDeviceControl::INotifyPointingCoordinatesReceived
	{
		public:
			//create a exos controller using a reference of a particular serial implementation.
			ExosIIMountControl(InterfaceType& interfaceImplementation) : 
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>(interfaceImplementation,*this),
				mTelescopeState(TelescopeMountState::Disconnected)
			{
				SerialDeviceControl::EquatorialCoordinates initialCoordinates;
				initialCoordinates.RightAscension = std::numeric_limits<float>::quiet_NaN();
				initialCoordinates.Declination = std::numeric_limits<float>::quiet_NaN();
				mCurrentPointingCoordinates.Set(initialCoordinates);
			}
			
			virtual ~ExosIIMountControl()
			{
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::Stop();
			}
			
			//open the serial connection and start the serial reporting.
			virtual void Start()
			{
				mTelescopeState.Set(TelescopeMountState::Unknown);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::Start();
			}
			
			//Stop the serial reporting and close the serial port.
			virtual void Stop()
			{
				mTelescopeState.Set(TelescopeMountState::Disconnected);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::Stop();
			}
			
			//stop any motion of the telescope.
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
				if(SerialDeviceControl::SerialCommand::GetStopMotionCommandMessage(messageBuffer))
				{
					SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
					
					mTelescopeState.Set(TelescopeMountState::Idle);
				}
				else
				{
					//TODO: error message.
				}
			}
			
			//Order to telescope to go to the parking state.
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
				if(SerialDeviceControl::SerialCommand::GetParkCommandMessage(messageBuffer))
				{
					SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				
					mTelescopeState.Set(TelescopeMountState::ParkingIssued);
				}
				else
				{
					//TODO: Error message.
				}
			}
			
			//GoTo and track the sky position represented by the equatorial coordinates.
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
				if(SerialDeviceControl::SerialCommand::GetGotoCommandMessage(messageBuffer,rightAscension,declination))
				{
					SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				}
				else
				{
					//TODO: error message
				}
			}
			
			//Set the location of the telesope, using decimal latitude and longitude parameters. This does not change to state of the telescope.
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
				if(SerialDeviceControl::SerialCommand::GetSetSiteLocationCommandMessage(messageBuffer,latitude,longitude))
				{
					SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				}
				else
				{
					//TODO: error message
				}
			}
			
			//issue the set time command, using date and time parameters. This does not change the state of the telescope.
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
				if(SerialDeviceControl::SerialCommand::GetSetDateTimeCommandMessage(messageBuffer,year,month,day,hour,minute,second))
				{				
					SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				}
				else
				{
					//TODO:error message.
				}
			}

			//Called each time a pair of coordinates was received from the serial interface.
			virtual void OnPointingCoordinatesReceived(float right_ascension, float declination)
			{
				//std::cout << "Received data : RA: " << right_ascension << " DEC:" << declination << std::endl;
				
				SerialDeviceControl::EquatorialCoordinates lastCoordinates = GetPointingCoordinates();
				
				SerialDeviceControl::EquatorialCoordinates coordinatesReceived;
				coordinatesReceived.RightAscension = right_ascension;
				coordinatesReceived.Declination = declination;
				
				mCurrentPointingCoordinates.Set(coordinatesReceived);
				
				SerialDeviceControl::EquatorialCoordinates delta = SerialDeviceControl::EquatorialCoordinates::Delta(lastCoordinates,coordinatesReceived);
				float absDelta = SerialDeviceControl::EquatorialCoordinates::Absolute(delta);
				
				TelescopeMountState currentState = GetTelescopeState();
				
				switch(currentState)
				{
					case TelescopeMountState::Unknown:
						//change to the parked state, resolving the unknown state.
						if(std::isnan(absDelta)) 
						{
							break;
						}
						
						if(!std::isnan(absDelta) && (absDelta>0.0f))
						{
							//see if the telescope is moving initially -> previous/externally triggered motion. 
							if(absDelta>TRACK_SLEW_THRESHOLD)
							{
								mTelescopeState.Set(TelescopeMountState::Slewing);
							}
							else
							{
								mTelescopeState.Set(TelescopeMountState::Tracking);
							}	
						}
						else
						{
							//assume parked otherwise.
							mTelescopeState.Set(TelescopeMountState::Parked);
						}
					break;
					
					case TelescopeMountState::ParkingIssued:
						//The user decides to park the telescope.
						if(absDelta>0.0f)
						{
							mTelescopeState.Set(TelescopeMountState::SlewingToParkingPosition);
						}
						else
						{
							mTelescopeState.Set(TelescopeMountState::Parked);
						}
					break;
					
					case TelescopeMountState::SlewingToParkingPosition:
						//measure if the scope is moving to park position, or assume its parked if no motion was reported.
						if(absDelta>0.0f)
						{
							mTelescopeState.Set(TelescopeMountState::SlewingToParkingPosition);
						}
						else
						{
							mTelescopeState.Set(TelescopeMountState::Parked);
						}
					break;
					
					case TelescopeMountState::Parked:
					case TelescopeMountState::Idle:
					case TelescopeMountState::TrackingIssued:
						if(absDelta>0.0f)
						{
							//may be externally triggered motion
							if(absDelta>TRACK_SLEW_THRESHOLD)
							{
								mTelescopeState.Set(TelescopeMountState::Slewing);
							}
							else
							{
								mTelescopeState.Set(TelescopeMountState::Tracking);
							}	
						}
					break;

					case TelescopeMountState::Tracking:
					case TelescopeMountState::Slewing:
						if(absDelta>0.0f)
						{
							//may be externally triggered motion
							if(absDelta>TRACK_SLEW_THRESHOLD)
							{
								mTelescopeState.Set(TelescopeMountState::Slewing);
							}
							else
							{
								mTelescopeState.Set(TelescopeMountState::Tracking);
							}	
						}
						else
						{
							mTelescopeState.Set(TelescopeMountState::Idle);
						}
					break;
					
					default:
						
					break;
				}
			}
			
			//return the current telescope state.
			TelescopeMountState GetTelescopeState()
			{
				return mTelescopeState.Get();
			}
			
			//return the current pointing coordinates.
			SerialDeviceControl::EquatorialCoordinates GetPointingCoordinates()
			{
				return mCurrentPointingCoordinates.Get();
			}
			
		private:
			//mutex protected container for the current coordinates the telescope is pointing at.
			SerialDeviceControl::CriticalData<SerialDeviceControl::EquatorialCoordinates> mCurrentPointingCoordinates;
			
			//mutex protected container for the current telescope state.
			SerialDeviceControl::CriticalData<TelescopeMountState> mTelescopeState;
	};
}
#endif
