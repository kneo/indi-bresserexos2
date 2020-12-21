#include "BresserExosIIGoToDriver.hpp"

using namespace GoToDriver;

static std::unique_ptr<BresserExosIIDriver> driver_instance(new BresserExosIIDriver());

void ISGetProperties(const char* dev)
{
	driver_instance->ISGetProperties(dev);
}

void ISNewSwitch(const char* dev, const char* name, ISState * states, char* names[], int n)
{
	driver_instance->ISNewSwitch(dev,name,states,names,n);
}

//TODO: this seems to have changed in indi 1.8.8
//void ISNewText(const char* dev, const char* name, ISState * states, char* names[], int n)
void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
	driver_instance->ISNewText(dev,name,texts,names,n);
}

//TODO: this seems to have changed in indi 1.8.8
//void ISNewNumber(const char* dev, const char* name, ISState * states, char* names[], int n)
void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
	driver_instance->ISNewNumber(dev,name,values,names,n);
}

void ISNewBLOB(const char* dev, const char* name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
	//driver_instance->ISNewBLOB(dev,name,sizes,blobs,formats,names,n);
	INDI_UNUSED(dev);
	INDI_UNUSED(name);
	INDI_UNUSED(sizes);
	INDI_UNUSED(blobsizes);
	INDI_UNUSED(blobs);
	INDI_UNUSED(formats);
	INDI_UNUSED(names);
	INDI_UNUSED(n);
}

void ISSnoopDevice(XMLEle* root)
{
	driver_instance->ISSnoopDevice(root);
}

//default constructor.
//sets the scope abilities, and default settings.
BresserExosIIDriver::BresserExosIIDriver() :
	mInterfaceWrapper(),
	mMountControl(mInterfaceWrapper)
{
	setVersion(BresserExosIIGoToDriverForIndi_VERSION_MAJOR, BresserExosIIGoToDriverForIndi_VERSION_MINOR);
	
	DBG_SCOPE = INDI::Logger::getInstance().addDebugLevel("Scope Verbose", "SCOPE");
	
	SetTelescopeCapability(TELESCOPE_CAN_PARK | TELESCOPE_CAN_GOTO | TELESCOPE_CAN_SYNC | TELESCOPE_CAN_ABORT |
                           TELESCOPE_HAS_TIME | TELESCOPE_HAS_LOCATION | TELESCOPE_CAN_CONTROL_TRACK,
                           0);
                           
    setDefaultPollingPeriod(500);
}

//destructor, not much going on here. Since most of the memory is statically allocated, there is not much to clean up.
BresserExosIIDriver::~BresserExosIIDriver()
{
	
}

//initialize the properties of the scope.
bool BresserExosIIDriver::initProperties()
{
	INDI::Telescope::initProperties();
	setTelescopeConnection(CONNECTION_SERIAL);
    
    addDebugControl();
    
    SetParkDataType(PARK_NONE);
    
    TrackState = SCOPE_IDLE;
    
    return true;
}

//update the properties of the scope visible in the EKOS dialogs for instance.
bool BresserExosIIDriver::updateProperties()
{
	bool rc = INDI::Telescope::updateProperties();
	
	return true;
}

//Connect to the scope, and ready everything for serial data exchange.
bool BresserExosIIDriver::Connect()
{
	bool rc = INDI::Telescope::Connect();
	
	LOGF_INFO("BresserExosIIDriver::Connect: Initializing ExosII GoTo on FD %d...",PortFD);
	
	mInterfaceWrapper.SetFD(PortFD);
	
	return true;
}

//Start the serial receiver thread, so the mount can report its pointing coordinates.
bool BresserExosIIDriver::Handshake()
{
	bool rc = INDI::Telescope::Handshake();

	LOGF_INFO("BresserExosIIDriver::Handshake: Starting Receiver Thread on FD %d...",PortFD);

	mMountControl.Start();
	//this message reports back the site location, also starts position reports, without changing anything on the scope.
	mMountControl.RequestSiteLocation();

	return true;
}

//Disconnect from the mount, and disable serial transmission.
bool BresserExosIIDriver::Disconnect()
{
	bool rc = INDI::Telescope::Disconnect();
	
	mMountControl.DisconnectSerial();
	
	mMountControl.Stop();
	
	LOG_INFO("BresserExosIIDriver::Disconnect: disabling pointing reporting, disconnected from scope. Bye!");

	return true;
}

//Return the name of the device, displayed in the e.g. EKOS dialogs
const char* BresserExosIIDriver::getDefaultName()
{
	return "Bresser Exos II GoTo Driver (for Firmware V2.3)";
}

//Periodically polled function to update the state of the driver, and synchronize it with the mount.
bool BresserExosIIDriver::ReadScopeStatus()
{
	SerialDeviceControl::EquatorialCoordinates currentCoordinates = mMountControl.GetPointingCoordinates();
	
	//LOGF_INFO("BresserExosIIDriver::ReadScopeStatus: Pointing to Right Ascension: %f Declination :%f...",currentCoordinates.RightAscension,currentCoordinates.Declination);
	
	NewRaDec(currentCoordinates.RightAscension, currentCoordinates.Declination);
	
	TelescopeMountControl::TelescopeMountState currentState = mMountControl.GetTelescopeState();
	
	switch(currentState)
	{
		case TelescopeMountControl::TelescopeMountState::Disconnected:
			//std::cout << "Disconnected" << std::endl;
			TrackState = SCOPE_IDLE;
		break;
		
		case TelescopeMountControl::TelescopeMountState::Unknown:
			//std::cout << "Unknown" << std::endl;
			TrackState = SCOPE_IDLE;
		break;
		
		case TelescopeMountControl::TelescopeMountState::ParkingIssued:
			//std::cout << "Parking command was issued" << std::endl;
			TrackState = SCOPE_PARKING;
		break;					

		case TelescopeMountControl::TelescopeMountState::SlewingToParkingPosition:
			//std::cout << "Slewing to parking position" << std::endl;
			TrackState = SCOPE_PARKING;
		break;					
		
		case TelescopeMountControl::TelescopeMountState::Parked:
			//std::cout << "Parked" << std::endl;
			TrackState = SCOPE_PARKED;
		break;
		
		case TelescopeMountControl::TelescopeMountState::Idle:
			//std::cout << "Idle" << std::endl;
			TrackState = SCOPE_IDLE;
		break;
		
		case TelescopeMountControl::TelescopeMountState::Slewing:
			//std::cout << "Slewing" << std::endl;
			TrackState = SCOPE_SLEWING;
		break;
		
		case TelescopeMountControl::TelescopeMountState::TrackingIssued:
			//std::cout << "Tracking issued" << std::endl;
			TrackState = SCOPE_TRACKING;
		break;
		
		case TelescopeMountControl::TelescopeMountState::Tracking:
			//std::cout << "Tracking" << std::endl;
			TrackState = SCOPE_TRACKING;
		break;
	}
	
	return true;
}

bool BresserExosIIDriver::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
	return INDI::Telescope::ISNewNumber(dev, name, values, names, n);
}

bool BresserExosIIDriver::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
	return false;
}

//Park the telescope. This will slew the telescope to the parking position == home position.
bool BresserExosIIDriver::Park()
{
	mMountControl.ParkPosition();
	SetParked(true);
	
	return true;
}

//Set the state of the driver to unpark allowing the scope to be manipulated again.			
bool BresserExosIIDriver::UnPark()
{
	SetParked(false);
	
	return true;
}

//Sync the astro software and mount coordinates.
bool BresserExosIIDriver::Sync(double ra, double dec)
{
	if(TrackState != SCOPE_TRACKING)
	{
		LOG_INFO("BresserExosIIDriver::Sync: Unable to Syncronize! This function only works when tracking a sky object!");
		return false;
	}
	
	LOGF_INFO("BresserExosIIDriver::Sync: Syncronizing to Right Ascension: %f Declination :%f...",ra,dec);
	
	mMountControl.Sync((float)ra,(float)dec);
	
	return true;
}

//Go to the coordinates in the sky, This automatically tracks the selected coordinates. 		
bool BresserExosIIDriver::Goto(double ra, double dec)
{
	LOGF_INFO("BresserExosIIDriver::Goto: Going to Right Ascension: %f Declination :%f...",ra,dec);
	
	mMountControl.GoTo((float)ra,(float)dec);		
	
	//INDI::Telescope::Goto(ra,dec);
	
	return true;
}

//Abort any motion of the telescope. This is state indipendent, and always possible when connected.
bool BresserExosIIDriver::Abort()
{
	//INDI::Telescope::Abort();
	LOG_INFO("BresserExosIIDriver::Abort: motion stopped!");
	
	mMountControl.StopMotion();
	return true;
}

//Set the tracking state of the scope, it either goes to the current coordinates or stops the scope motion.
bool BresserExosIIDriver::SetTrackingEnabled(bool enabled)
{
	if(enabled)
	{
		SerialDeviceControl::EquatorialCoordinates currentCoordinates = mMountControl.GetPointingCoordinates();
		
		LOGF_INFO("BresserExosIIDriver::SetTrackingEnabled: Tracking to Right Ascension: %f Declination :%f...",currentCoordinates.RightAscension,currentCoordinates.Declination);
	
		mMountControl.GoTo(currentCoordinates.RightAscension, currentCoordinates.Declination);
	}
	else
	{
		mMountControl.StopMotion();
	}

	return true;
}

//update the time of the scope.
bool BresserExosIIDriver::updateTime(ln_date *utc, double utc_offset)
{
	INDI::Telescope::updateTime(utc,utc_offset);
	
	uint16_t years = (uint16_t)utc->years;
	uint8_t months = (uint8_t)utc->months;
	uint8_t days =   (uint8_t)utc->days;
	
	uint8_t hours =   (uint8_t) utc->hours;
	uint8_t minutes = (uint8_t) utc->minutes;
	uint8_t seconds = (uint8_t) utc->seconds;
	
	LOGF_INFO("Date/Time updated: %d:%d:%d %d-%d-%d", hours, minutes, seconds, years, months, days);
	
	mMountControl.SetDateTime(years,months,days,hours,minutes,seconds);
	
	return true;
}

//update the location of the scope.
bool BresserExosIIDriver::updateLocation(double latitude, double longitude, double elevation)
{
	//INDI::Telescope::updateLocation(latitude,longitude,elevation);
	
	INDI_UNUSED(elevation);
	
	LOGF_INFO("Location updated: Longitude (%g) Latitude (%g)", longitude, latitude);
	
	mMountControl.SetSiteLocation((float)latitude,(float) longitude);
	
	return true;
}
