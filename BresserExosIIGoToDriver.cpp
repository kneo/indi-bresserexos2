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

BresserExosIIDriver::BresserExosIIDriver() :
	mInterfaceWrapper(),
	mMountControl(mInterfaceWrapper)
{
	setVersion(BresserExosIIGoToDriverForIndi_VERSION_MAJOR, BresserExosIIGoToDriverForIndi_VERSION_MINOR);
	
	DBG_SCOPE = INDI::Logger::getInstance().addDebugLevel("Scope Verbose", "SCOPE");
	
	SetTelescopeCapability(TELESCOPE_CAN_PARK | TELESCOPE_CAN_GOTO | TELESCOPE_CAN_ABORT |
                           TELESCOPE_HAS_TIME | TELESCOPE_HAS_LOCATION | TELESCOPE_CAN_CONTROL_TRACK,
                           0);
                           
    setDefaultPollingPeriod(500);
}


BresserExosIIDriver::~BresserExosIIDriver()
{
	
}

 bool BresserExosIIDriver::initProperties()
{
	INDI::Telescope::initProperties();
	setTelescopeConnection(CONNECTION_SERIAL);
    
    addDebugControl();
    
    SetParkDataType(PARK_RA_DEC);
    
    TrackState = SCOPE_IDLE;
    
    return true;
}

bool BresserExosIIDriver::updateProperties()
{

	
	bool rc = INDI::Telescope::updateProperties();
	
	return true;
}

bool BresserExosIIDriver::Connect()
{
	bool rc = INDI::Telescope::Connect();
	
	LOGF_INFO("BresserExosIIDriver::Connect: Initializing ExosII GoTo on FD %d...",PortFD);
	
	mInterfaceWrapper.SetFD(PortFD);
	
	return true;
}

bool BresserExosIIDriver::Handshake()
{
	bool rc = INDI::Telescope::Handshake();

	LOGF_INFO("BresserExosIIDriver::Handshake: Starting Receiver Thread on FD %d...",PortFD);

	mMountControl.Start();

	return true;
}

bool BresserExosIIDriver::Disconnect()
{
	bool rc = INDI::Telescope::Disconnect();
	
	mMountControl.Stop();

	return true;
}

const char* BresserExosIIDriver::getDefaultName()
{
	return "Bresser Exos II GoTo Driver (for Firmware V2.3)";
}

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

bool BresserExosIIDriver::Park()
{
	mMountControl.ParkPosition();
	SetParked(true);
	
	return true;
}
			
bool BresserExosIIDriver::UnPark()
{
	SetParked(false);
	
	return true;
}

bool BresserExosIIDriver::Sync(double ra, double dec)
{
	NewRaDec(ra, dec);
	
	return true;
}
			
bool BresserExosIIDriver::Goto(double ra, double dec)
{
	LOGF_INFO("BresserExosIIDriver::Goto: Going to Right Ascension: %f Declination :%f...",ra,dec);
	
	mMountControl.GoTo((float)ra,(float)dec);		
	
	//INDI::Telescope::Goto(ra,dec);
	
	return true;
}
			
bool BresserExosIIDriver::Abort()
{
	//INDI::Telescope::Abort();
	
	mMountControl.StopMotion();
	return true;
}

bool BresserExosIIDriver::updateTime(ln_date *utc, double utc_offset)
{
	INDI::Telescope::updateTime(utc,utc_offset);
	
	uint16_t years = (uint16_t)utc->years;
	uint8_t months = (uint8_t)utc->months;
	uint8_t days =   (uint8_t)utc->days;
	
	uint8_t hours =   (uint8_t) utc->hours;
	uint8_t minutes = (uint8_t) utc->minutes;
	uint8_t seconds = (uint8_t) utc->seconds;
	
	mMountControl.SetDateTime(years,months,days,hours,minutes,seconds);
	
	return true;
}
			
bool BresserExosIIDriver::updateLocation(double latitude, double longitude, double elevation)
{
	//INDI::Telescope::updateLocation(latitude,longitude,elevation);
	
	INDI_UNUSED(elevation);
	
	LOGF_INFO("Location updated: Longitude (%g) Latitude (%g)", longitude, latitude);
	
	mMountControl.SetSiteLocation((float)latitude,(float) longitude);
	
	
	return true;
}
