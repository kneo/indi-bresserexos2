#include "SerialCommandTransceiver.hpp"

using SerialDeviceControl::SerialCommandTransceiver;

SerialCommandTransceiver::SerialCommandTransceiver(ISerialInterface* interfaceImplementation,INotifyPointingCoordinatesReceived* callback) : 
mDataReceivedCallback(callback),
mInterfaceImplementation(interfaceImplementation),
mThreadRunning(false)
{
	
}

SerialCommandTransceiver::~SerialCommandTransceiver()
{

}

void SerialCommandTransceiver::Start()
{
	
}

void SerialCommandTransceiver::SendMessageBuffer(uint8_t* buffer, size_t offset, size_t length)
{
	
}

void SerialCommandTransceiver::Stop()
{
	mThreadRunning.Set(false);
	
	//TODO: implement more forceful termination if necessary.
}

void SerialCommandTransceiver::SerialReaderThreadFunction()
{
	if(mInterfaceImplementation==nullptr)
	{
		//TODO: log error: invalid implementation
		return;
	}
	
	if(mDataReceivedCallback==nullptr)
	{
		//TODO: log error: no receive callback.
		return;
	}
	
	bool running = mThreadRunning.Get();
	
	if(running==false)
	{
		mThreadRunning.Set(true);
		
		do
		{
			
			//Do serial business
			
			running = mThreadRunning.Get();
		}
		while(running == true);
		
	}
}

void SerialCommandTransceiver::TryParseMessagesFromBuffer()
{
	
}
