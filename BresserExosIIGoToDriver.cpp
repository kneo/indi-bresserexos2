#include "BresserExosIIGoToDriver.hpp"

using namespace GoToDriver;
using namespace SerialDeviceControl;

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
                           TELESCOPE_HAS_TIME | TELESCOPE_HAS_LOCATION | TELESCOPE_CAN_CONTROL_TRACK, 0);
                           
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

    initGuiderProperties(getDeviceName(), MOTION_TAB);
    
    SetParkDataType(PARK_NONE);

    TrackState = SCOPE_IDLE;
    
    addAuxControls();
    
    setDriverInterface(getDriverInterface() | GUIDER_INTERFACE);

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

	mMountControl.RequestSiteLocation();

	//this message reports back the site location, also starts position reports, without changing anything on the scope.
	
	return true;
}

//Start the serial receiver thread, so the mount can report its pointing coordinates.
bool BresserExosIIDriver::Handshake()
{
	LOGF_INFO("BresserExosIIDriver::Handshake: Starting Receiver Thread on FD %d...",PortFD);

	mInterfaceWrapper.SetFD(PortFD);
	
	mMountControl.Start();
	
	bool rc = INDI::Telescope::Handshake();

	return true;
}

//Disconnect from the mount, and disable serial transmission.
bool BresserExosIIDriver::Disconnect()
{
	//mMountControl.DisconnectSerial();
	
	mMountControl.Stop();
	
	LOG_INFO("BresserExosIIDriver::Disconnect: disabling pointing reporting, disconnected from scope. Bye!");
	
	bool rc = INDI::Telescope::Disconnect();

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
	NewRaDec(currentCoordinates.RightAscension, currentCoordinates.Declination);
	
	TelescopeMountControl::TelescopeMountState currentState = mMountControl.GetTelescopeState();
	std::string stringState = "";
	switch(currentState)
	{
		case TelescopeMountControl::TelescopeMountState::Disconnected:
			//std::cout << "Disconnected" << std::endl;
			TrackState = SCOPE_IDLE;
			stringState = EXPR_TO_STRING(SCOPE_IDLE);
		break;
		
		case TelescopeMountControl::TelescopeMountState::Unknown:
			//std::cout << "Unknown" << std::endl;
			TrackState = SCOPE_IDLE;
			stringState = EXPR_TO_STRING(SCOPE_IDLE);
		break;
		
		case TelescopeMountControl::TelescopeMountState::ParkingIssued:
			//std::cout << "Parking command was issued" << std::endl;
			TrackState = SCOPE_PARKING;
			stringState = EXPR_TO_STRING(SCOPE_PARKING);
		break;						
		
		case TelescopeMountControl::TelescopeMountState::Parked:
			//std::cout << "Parked" << std::endl;
			TrackState = SCOPE_PARKED;
			stringState = EXPR_TO_STRING(SCOPE_PARKED);
		break;
		
		case TelescopeMountControl::TelescopeMountState::Idle:
			//std::cout << "Idle" << std::endl;
			TrackState = SCOPE_IDLE;
			stringState = EXPR_TO_STRING(SCOPE_IDLE);
		break;
		
		case TelescopeMountControl::TelescopeMountState::Slewing:
			//std::cout << "Slewing" << std::endl;
			TrackState = SCOPE_SLEWING;
			stringState = EXPR_TO_STRING(SCOPE_SLEWING);
		break;

		case TelescopeMountControl::TelescopeMountState::Tracking:
			//std::cout << "Tracking" << std::endl;
			TrackState = SCOPE_TRACKING;
			stringState = EXPR_TO_STRING(SCOPE_TRACKING);
		break;
		
		case TelescopeMountControl::TelescopeMountState::MoveWhileTracking:
			TrackState = SCOPE_TRACKING;
			stringState = EXPR_TO_STRING(SCOPE_TRACKING);
		break;
		
		default:
			stringState = "unknown";
		break;
	}
	
	//LOGF_INFO("BresserExosIIDriver::ReadScopeStatus: %s ...",stringState);
	
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


bool BresserExosIIDriver::MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command)
{
	if (TrackState != SCOPE_TRACKING)
    {
        LOG_ERROR("Error: this command only works while tracking.");
        return false;
    }
    
    SerialDeviceControl::SerialCommandID direction;
    
    switch(dir)
    {
		case DIRECTION_NORTH:
			direction = SerialDeviceControl::SerialCommandID::MOVE_NORTH_COMMAND_ID;
		break;
		
		case DIRECTION_SOUTH:
			direction = SerialDeviceControl::SerialCommandID::MOVE_SOUTH_COMMAND_ID;
		break;
		
		default:
			LOG_ERROR("Error: invalid direction value!");
			return false;
	}
	
	switch(command)
	{
		case MOTION_START:
			mMountControl.StartMotionToDirection(direction,10);
			return true;
		
		case MOTION_STOP:
			mMountControl.StopMotionToDirection();
			return true;
		
		default:
		
		break;
	}
	
    return false;
}

bool BresserExosIIDriver::MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command)
{
	if (TrackState != SCOPE_TRACKING)
    {
        LOG_ERROR("Error: this command only works while tracking.");
        return false;
    }
    
    SerialDeviceControl::SerialCommandID direction;
    
    switch(dir)
    {
		case DIRECTION_EAST:
			direction = SerialDeviceControl::SerialCommandID::MOVE_EAST_COMMAND_ID;
		break;
		
		case DIRECTION_WEST:
			direction = SerialDeviceControl::SerialCommandID::MOVE_WEST_COMMAND_ID;
		break;
		
		default:
			LOG_ERROR("Error: invalid direction value!");
			return false;
	}
	
	switch(command)
	{
		case MOTION_START:
			mMountControl.StartMotionToDirection(direction,10);
			return true;
		
		case MOTION_STOP:
			mMountControl.StopMotionToDirection();
			return true;
		
		default:
		
		break;
	}
	
	return false;
}

//TODO: find out amount of degree change per "pulse command" -> ?
//amount of time necessary to transmit a message -> 135 µs
IPState BresserExosIIDriver::GuideNorth(uint32_t ms)
{
    /*long timetaken_us;
    int timeremain_ms;

    // If already moving, then stop movement
    if (MovementNSSP.s == IPS_BUSY)
    {
        int dir = IUFindOnSwitchIndex(&MovementNSSP);
        MoveNS(dir == 0 ? DIRECTION_NORTH : DIRECTION_SOUTH, MOTION_STOP);
    }

    if (GuideNSTID)
    {
        IERmTimer(GuideNSTID);
        GuideNSTID = 0;
    }

    start_pmc8_guide(PortFD, PMC8_N, (int)ms, timetaken_us);

    timeremain_ms = (int)(ms - ((float)timetaken_us) / 1000.0);

    if (timeremain_ms < 0)
        timeremain_ms = 0;

    GuideNSTID = IEAddTimer(timeremain_ms, guideTimeoutHelperN, this);*/

    return IPS_BUSY;
}

IPState BresserExosIIDriver::GuideSouth(uint32_t ms)
{
    /*long timetaken_us;
    int timeremain_ms;

    // If already moving, then stop movement
    if (MovementNSSP.s == IPS_BUSY)
    {
        int dir = IUFindOnSwitchIndex(&MovementNSSP);
        MoveNS(dir == 0 ? DIRECTION_NORTH : DIRECTION_SOUTH, MOTION_STOP);
    }

    if (GuideNSTID)
    {
        IERmTimer(GuideNSTID);
        GuideNSTID = 0;
    }

    start_pmc8_guide(PortFD, PMC8_S, (int)ms, timetaken_us);

    timeremain_ms = (int)(ms - ((float)timetaken_us) / 1000.0);

    if (timeremain_ms < 0)
        timeremain_ms = 0;

    GuideNSTID      = IEAddTimer(timeremain_ms, guideTimeoutHelperS, this);*/

    return IPS_BUSY;
}

IPState BresserExosIIDriver::GuideEast(uint32_t ms)
{
    /*long timetaken_us;
    int timeremain_ms;

    // If already moving (no pulse command), then stop movement
    if (MovementWESP.s == IPS_BUSY)
    {
        int dir = IUFindOnSwitchIndex(&MovementWESP);
        MoveWE(dir == 0 ? DIRECTION_WEST : DIRECTION_EAST, MOTION_STOP);
    }

    if (GuideWETID)
    {
        IERmTimer(GuideWETID);
        GuideWETID = 0;
    }

    start_pmc8_guide(PortFD, PMC8_E, (int)ms, timetaken_us);

    timeremain_ms = (int)(ms - ((float)timetaken_us) / 1000.0);

    if (timeremain_ms < 0)
        timeremain_ms = 0;

    GuideWETID      = IEAddTimer(timeremain_ms, guideTimeoutHelperE, this);*/
    return IPS_BUSY;
}

IPState BresserExosIIDriver::GuideWest(uint32_t ms)
{
    /*long timetaken_us;
    int timeremain_ms;

    // If already moving (no pulse command), then stop movement
    if (MovementWESP.s == IPS_BUSY)
    {
        int dir = IUFindOnSwitchIndex(&MovementWESP);
        MoveWE(dir == 0 ? DIRECTION_WEST : DIRECTION_EAST, MOTION_STOP);
    }

    if (GuideWETID)
    {
        IERmTimer(GuideWETID);
        GuideWETID = 0;
    }

    start_pmc8_guide(PortFD, PMC8_W, (int)ms, timetaken_us);

    timeremain_ms = (int)(ms - ((float)timetaken_us) / 1000.0);

    if (timeremain_ms < 0)
        timeremain_ms = 0;

    GuideWETID      = IEAddTimer(timeremain_ms, guideTimeoutHelperW, this);*/
    return IPS_BUSY;
}

void BresserExosIIDriver::guideTimeout(/*PMC8_DIRECTION calldir*/)
{
    // end previous pulse command
    /*stop_pmc8_guide(PortFD, calldir);

    if (calldir == PMC8_N || calldir == PMC8_S)
    {
        GuideNSNP.np[0].value = 0;
        GuideNSNP.np[1].value = 0;
        GuideNSNP.s           = IPS_IDLE;
        GuideNSTID            = 0;
        IDSetNumber(&GuideNSNP, nullptr);
    }
    if (calldir == PMC8_W || calldir == PMC8_E)
    {
        GuideWENP.np[0].value = 0;
        GuideWENP.np[1].value = 0;
        GuideWENP.s           = IPS_IDLE;
        GuideWETID            = 0;
        IDSetNumber(&GuideWENP, nullptr);
    }

    LOG_DEBUG("GUIDE CMD COMPLETED");*/
}

//GUIDE The timer helper functions.
void BresserExosIIDriver::guideTimeoutHelperN(void *p)
{
    
}
void BresserExosIIDriver::guideTimeoutHelperS(void *p)
{
   
}
void BresserExosIIDriver::guideTimeoutHelperW(void *p)
{
    
}
void BresserExosIIDriver::guideTimeoutHelperE(void *p)
{
    
}
