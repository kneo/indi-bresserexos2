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
#include <mutex>
#include <condition_variable>
#include <thread>
#include "Config.hpp"

#include "SerialDeviceControl/CriticalData.hpp"
#include "SerialDeviceControl/SerialCommand.hpp"
#include "SerialDeviceControl/SerialCommandTransceiver.hpp"
#include "SerialDeviceControl/INotifyPointingCoordinatesReceived.hpp"

//The manual states a tracking speed for 0.004°/s everything above is considered slewing.
#define TRACK_SLEW_THRESHOLD (0.0045)

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
		//The driver received at least one response from the mount,
		//indicating the communication is working, and the mount accepts commands.
		Connected = 2,
		//If the user issues the park command the telescope likely needs to slew to the parking position, 
		//this is determined by the differentials of the positions information send by the controller.
		ParkingIssued = 3,
		//if parking was issued and the telescope is found moving this state is active.
		SlewingToParkingPosition = 4,
		//If the status report messages arrive and the telescope is not moving, this is the assumed state.
		//the telescope is assumed in park/initial position according to manual
		//or if the "park" command is issued.
		Parked = 5,
		//The controller autonomously desides the motion speeds, and does not report any "state".
		//This state is assumed if the motion speeds exceed a certain threshold.
		Slewing = 6,
		//The user ordered the telescope to track/goto a specific location. State of motion has to be determined by the report message.
		TrackingIssued = 7,
		//This state is assumed if the telescope is moves below the slewing threshold.
		//its also the default when a "goto" is issued, since the telescope controller automatically tracks the issued coordinates.
		Tracking = 8,
		//when tracking an object, move to a direction.
		MoveWhileTracking = 9,
		//This state is reached when issuing the stop command while slewing or tracking.
		Idle = 10,
	};
	
	//
	struct MotionState
	{
		//move in what direction
		SerialDeviceControl::SerialCommandID MotionDirection;
		//how many messages per second
		uint16_t CommandsPerSecond;
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
				mTelescopeState(TelescopeMountState::Disconnected),
				mIsMotionControlThreadRunning(false),
				mIsMotionControlRunning(false)
			{
				SerialDeviceControl::EquatorialCoordinates initialCoordinates;
				initialCoordinates.RightAscension = std::numeric_limits<float>::quiet_NaN();
				initialCoordinates.Declination    = std::numeric_limits<float>::quiet_NaN();
				
				mCurrentPointingCoordinates.Set(initialCoordinates);
				mSiteLocationCoordinates.Set(initialCoordinates);
				
				MotionState initialState;
				initialState.MotionDirection = SerialDeviceControl::SerialCommandID::NULL_COMMAND_ID;
				initialState.CommandsPerSecond = 0;
				
				mMotionState.Set(initialState);
			}
			
			virtual ~ExosIIMountControl()
			{
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::Stop();
			}
			
			//open the serial connection and start the serial reporting.
			virtual bool Start()
			{
				mTelescopeState.Set(TelescopeMountState::Unknown);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::Start();
				
				mMotionCommandThread = std::thread(&ExosIIMountControl<InterfaceType>::MotionControlThreadFunction,this);
				
				return true;
			}
			
			//Stop the serial reporting and close the serial port.
			virtual bool Stop()
			{
				if(mIsMotionControlThreadRunning.Get())
				{
					mIsMotionControlThreadRunning.Set(false);
					StopMotionToDirection();
					mMotionCommandThread.join();
				}

				DisconnectSerial();
				
				mTelescopeState.Set(TelescopeMountState::Disconnected);
				
				SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::Stop();
				
				return true;
			}
			
			bool StartMotionToDirection(SerialDeviceControl::SerialCommandID direction,uint16_t commandsPerSecond)
			{
				//this only works while tracking a target
				switch(mTelescopeState.Get())
				{
					case TelescopeMountState::Tracking:
					break;
					
					case TelescopeMountState::MoveWhileTracking:
						std::cerr << "Error: already moving, stop motion first before issuing another command." << std::endl;
						return false;
					
					default:
						std::cerr << "Error: motion is only available while tracking." << std::endl;
						return false;
				}
				

				{
					std::lock_guard<std::mutex> notifyLock(mMotionCommandControlMutex);
				
					MotionState initialState;
					initialState.MotionDirection = direction;
					initialState.CommandsPerSecond = commandsPerSecond;
				
					mMotionState.Set(initialState);
				
					mIsMotionControlRunning.Set(true);
				}
				
				mMotionControlCondition.notify_all();
				
				return true;
			}
			
			bool StopMotionToDirection()
			{
				/*if(mTelescopeState.Get()!=TelescopeMountState::MoveWhileTracking)
				{
					std::cerr << "INFO: motion already disabled." << std::endl;
					return;
				}
				
				mTelescopeState.Set(TelescopeMountState::Tracking);*/
				
				//this changes back to tracking
				MotionState stopState;
				stopState.MotionDirection = SerialDeviceControl::SerialCommandID::NULL_COMMAND_ID;
				stopState.CommandsPerSecond = 0;
				
				mMotionState.Set(stopState);
				
				mIsMotionControlRunning.Set(false);
				
				mMotionControlCondition.notify_all();
				
				return true;
			}
			
			//Disconnect from the mount.
			bool DisconnectSerial()
			{
				/*TelescopeMountState currentState = mTelescopeState.Get();
				
				if(currentState<TelescopeMountState::Unknown)
				{
					//TODO: error invalid state.
					mTelescopeState.Set(TelescopeMountState::Disconnected);
					std::cerr << "Error: can not send disconnect, not connected." << std::endl;
					return false;
				}*/
				
				std::vector<uint8_t> messageBuffer;
				if(SerialDeviceControl::SerialCommand::GetDisconnectCommandMessage(messageBuffer))
				{
					return SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
					
					//smTelescopeState.Set(TelescopeMountState::Disconnected);
				}
				else
				{
					//TODO: error message.
					return false;
				}
			}
			
			//stop any motion of the telescope.
			bool StopMotion()
			{
				/*switch(mTelescopeState.Get())
				{
					case TelescopeMountState::Unknown:
						std::cerr << "Error: can not send stop, not connected." << std::endl;
						return;
					
					case TelescopeMountState::MoveWhileTracking:
						StopMotionToDirection();
					break;
					
				}*/
				
				std::vector<uint8_t> messageBuffer;
				if(SerialDeviceControl::SerialCommand::GetStopMotionCommandMessage(messageBuffer))
				{
					return SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
					
					//mTelescopeState.Set(TelescopeMountState::Idle);
				}
				else
				{
					//TODO: error message.
					return false;
				}
			}
			
			//Order to telescope to go to the parking state.
			bool ParkPosition()
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				
				/*if(currentState<TelescopeMountState::Parked)
				{
					//TODO: error invalid state.
					std::cerr << "Error: can not park, not connected." << std::endl;
					return;
				}
				
				//StopMotionToDirection();*/
				
				std::vector<uint8_t> messageBuffer;
				if(SerialDeviceControl::SerialCommand::GetParkCommandMessage(messageBuffer))
				{
					return SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				
					//mTelescopeState.Set(TelescopeMountState::ParkingIssued);
				}
				else
				{
					//TODO: Error message.
					return false;
				}
			}
			
			//GoTo and track the sky position represented by the equatorial coordinates.
			bool GoTo(float rightAscension, float declination)
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				
				/*switch(mTelescopeState.Get())
				{
					case TelescopeMountState::MoveWhileTracking:
						StopMotionToDirection();
						
						break;
											
					default:
						break;
				}
				
				if(currentState<TelescopeMountState::Parked)
				{
					//TODO: error invalid state.
					std::cerr << "Error: can not goto, invalid state." << std::endl;
					return false;
				}*/
				
				std::vector<uint8_t> messageBuffer;
				if(SerialDeviceControl::SerialCommand::GetGotoCommandMessage(messageBuffer,rightAscension,declination))
				{
					return SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				}
				else
				{
					//TODO: error message
					return false;
				}
			}
			
			//GoTo and track the sky position represented by the equatorial coordinates.
			bool Sync(float rightAscension, float declination)
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				
				/*if(currentState != TelescopeMountState::Tracking)
				{
					//TODO: error invalid state.
					std::cerr << "Error: can not goto, invalid state." << std::endl;
					return false;
				}*/
				
				std::vector<uint8_t> messageBuffer;
				if(SerialDeviceControl::SerialCommand::GetSyncCommandMessage(messageBuffer,rightAscension,declination))
				{
					return SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				}
				else
				{
					//TODO: error message
					return false;
				}
			}
			
			//Set the location of the telesope, using decimal latitude and longitude parameters. 
			//This does not change to state of the telescope.
			bool SetSiteLocation(float latitude, float longitude)
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				
				/*if(currentState<TelescopeMountState::Parked)
				{
					//TODO: error invalid state.
					std::cerr << "Error: can not set site location, invalid telescope state." << std::endl;
					return false;
				}*/
				
				std::vector<uint8_t> messageBuffer;
				if(SerialDeviceControl::SerialCommand::GetSetSiteLocationCommandMessage(messageBuffer,latitude,longitude))
				{
					return SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				}
				else
				{
					//TODO: error message
					return false;
				}
			}
			
			//Set the location of the telesope, using decimal latitude and longitude parameters. 
			//This does not change to state of the telescope.
			bool RequestSiteLocation()
			{
				std::vector<uint8_t> messageBuffer;
				if(SerialDeviceControl::SerialCommand::GetGetSiteLocationCommandMessage(messageBuffer))
				{
					//std::cout << "Message sent!" << std::endl;
					return SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				}
				else
				{
					//TODO: error message
					return false;
				}
			}
			
			//issue the set time command, using date and time parameters. This does not change the state of the telescope.
			bool SetDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				//std::cout << "month: " << month << std::endl;
				
				/*if(currentState<TelescopeMountState::Parked)
				{
					//TODO: error invalid state.
					std::cerr << "Error: can not set date and time, invalid telescope state." << std::endl;
					return;
				}*/
				
				std::vector<uint8_t> messageBuffer;
				if(SerialDeviceControl::SerialCommand::GetSetDateTimeCommandMessage(messageBuffer,year,month,day,hour,minute,second))
				{				
					return SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				}
				else
				{
					//TODO:error message.
					return false;
				}
			}
			
			template<SerialDeviceControl::SerialCommandID Direction>
			bool MoveDirection()
			{
				TelescopeMountState currentState = mTelescopeState.Get();
				
				if(currentState!=TelescopeMountState::MoveWhileTracking)
				{
					//TODO: error invalid state.
					std::cerr << "Error: can not move telescope, invalid state. Track an object before using!" << std::endl;
					return false;
				}
				
				std::vector<uint8_t> messageBuffer;
				if(SerialDeviceControl::SerialCommand::GetMoveWhileTrackingCommandMessage(messageBuffer,Direction))
				{				
					return SerialDeviceControl::SerialCommandTransceiver<InterfaceType,TelescopeMountControl::ExosIIMountControl<InterfaceType>>::SendMessageBuffer(&messageBuffer[0],0,messageBuffer.size());
				}
				else
				{
					//TODO:error message.
					return false;
				}
			}
			
			bool MoveNorth()
			{
				return MoveDirection<SerialDeviceControl::SerialCommandID::MOVE_NORTH_COMMAND_ID>();
			}
			
			bool MoveSouth()
			{
				return MoveDirection<SerialDeviceControl::SerialCommandID::MOVE_SOUTH_COMMAND_ID>();
			}
			
			bool MoveEast()
			{
				return MoveDirection<SerialDeviceControl::SerialCommandID::MOVE_EAST_COMMAND_ID>();
			}
			
			bool MoveWest()
			{
				return MoveDirection<SerialDeviceControl::SerialCommandID::MOVE_WEST_COMMAND_ID>();
			}

			//Called each time a pair of coordinates was received from the serial interface.
			virtual void OnPointingCoordinatesReceived(float right_ascension, float declination)
			{
				//std::cerr << "Received data : RA: " << right_ascension << " DEC:" << declination << std::endl;
				
				SerialDeviceControl::EquatorialCoordinates lastCoordinates = GetPointingCoordinates();
				
				SerialDeviceControl::EquatorialCoordinates coordinatesReceived;
				coordinatesReceived.RightAscension = right_ascension;
				coordinatesReceived.Declination = declination;
				
				mCurrentPointingCoordinates.Set(coordinatesReceived);
				
				SerialDeviceControl::EquatorialCoordinates delta = SerialDeviceControl::EquatorialCoordinates::Delta(lastCoordinates,coordinatesReceived);
				float absDelta = SerialDeviceControl::EquatorialCoordinates::Absolute(delta);
				
				TelescopeMountState currentState = GetTelescopeState();
				//TODO: this state machine needs to be reconcidered!
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
			
			//Called each time a pair of geo coordinates was received from the serial inferface. This happends only by active request (GET_SITE_LOCATION_COMMAND_ID)
			virtual void OnSiteLocationCoordinatesReceived(float latitude, float longitude)
			{
				std::cerr << "Received data : LAT: " << latitude << " LON:" << longitude << std::endl;
				
				SerialDeviceControl::EquatorialCoordinates coordinatesReceived;
				coordinatesReceived.RightAscension = latitude;
				coordinatesReceived.Declination = longitude;
				
				mSiteLocationCoordinates.Set(coordinatesReceived);
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
			
			//return the current pointing coordinates.
			SerialDeviceControl::EquatorialCoordinates GetSiteLocation()
			{
				return mSiteLocationCoordinates.Get();
			}
			
		private:
			//mutex protected container for the current coordinates the telescope is pointing at.
			SerialDeviceControl::CriticalData<SerialDeviceControl::EquatorialCoordinates> mCurrentPointingCoordinates;
			
			//mutex protected container for the current site location set in the telescope.
			SerialDeviceControl::CriticalData<SerialDeviceControl::EquatorialCoordinates> mSiteLocationCoordinates;
			
			//mutex protected container for the current telescope state.
			SerialDeviceControl::CriticalData<TelescopeMountState> mTelescopeState;
			
			//mutex protected state variable of the motion thread.
			SerialDeviceControl::CriticalData<bool> mIsMotionControlThreadRunning;
			
			//mutex protected state variable of the motion thread to indicate if a motion is started.
			SerialDeviceControl::CriticalData<bool> mIsMotionControlRunning;
			
			//holds the state of motion, direction and rate.
			SerialDeviceControl::CriticalData<MotionState> mMotionState;
			
			//motion control thread structure, to periodically sent direction commands.
			std::thread mMotionCommandThread;
			
			//mutex used for signaling the thread to start motion command sending,
			//this is also used to halt the thread when the motion should be stopped.
			std::mutex mMotionCommandControlMutex;
			
			//Condition variable to signal start and stop of motion command sending.
			std::condition_variable mMotionControlCondition;
			
			void MotionControlThreadFunction()
			{
				bool isThreadRunning = mIsMotionControlThreadRunning.Get();
				bool isMotionRunning;// = mIsMotionControlRunning.Get();
				
				if(!isThreadRunning)
				{
					mIsMotionControlThreadRunning.Set(true);
					
					std::cerr << "Motion Control Thread started!" << std::endl;

					do
					{
						//std::cerr << "Motion Control main loop started!" << std::endl;
						do
						{
							//std::cerr << "Motion Control motion loop started!" << std::endl;
							std::unique_lock<std::mutex> motionLock(mMotionCommandControlMutex);
							isMotionRunning = mIsMotionControlRunning.Get();
							
							if(!isMotionRunning)
							{
								//std::cerr << "condition wait!" << std::endl;
								//initially no motion commands are send, so wait until a motion in either direction is started by the start call.
								mMotionControlCondition.wait(motionLock);
								//std::cerr << "condition triggered!" << std::endl;
								
								isMotionRunning = mIsMotionControlRunning.Get();
								if(!isMotionRunning)
								{
									//motionLock.unlock();
									break;
								}
							}
							
							MotionState motionState = mMotionState.Get();
							
							//check if motion state is valid.
							if
							(
								motionState.MotionDirection<=SerialDeviceControl::SerialCommandID::NULL_COMMAND_ID ||
								motionState.MotionDirection>=SerialDeviceControl::SerialCommandID::STOP_MOTION_COMMAND_ID ||
								motionState.CommandsPerSecond == 0
							)
							{
								//motion is tripped but no values are provided -> disable motion and wait again.
								mIsMotionControlRunning.Set(false);
								
								motionState.MotionDirection=SerialDeviceControl::SerialCommandID::NULL_COMMAND_ID;
								motionState.CommandsPerSecond=0;
								
								mMotionState.Set(motionState);
								//motionLock.unlock();
								break;
							}
							
							int waitTime = 1000 / motionState.CommandsPerSecond;
							
							//send command to move to direction.
							switch(motionState.MotionDirection)
							{
								case SerialDeviceControl::SerialCommandID::MOVE_EAST_COMMAND_ID:
									MoveEast();
								break;
								
								case SerialDeviceControl::SerialCommandID::MOVE_WEST_COMMAND_ID:
									MoveWest();
								break;
								
								case SerialDeviceControl::SerialCommandID::MOVE_NORTH_COMMAND_ID:
									MoveNorth();
								break;
								
								case SerialDeviceControl::SerialCommandID::MOVE_SOUTH_COMMAND_ID:
									MoveSouth();
								break;
								
								default:
									break;
							}
							
							//wait before next loop.
							std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(waitTime));
							
							isMotionRunning = mIsMotionControlRunning.Get();
							//motionLock.unlock();
						}
						while(isMotionRunning);
						
						isThreadRunning = mIsMotionControlThreadRunning.Get();
						//std::cerr << "Motion Control main loop stopped!" << std::endl;
					}
					while(isThreadRunning);
					
					std::cerr << "Motion Control Thread stopped!" << std::endl;
				}
			}
	};
}
#endif
