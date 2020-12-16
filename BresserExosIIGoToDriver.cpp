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
    
    return true;
}

bool BresserExosIIDriver::updateProperties()
{
	bool rc = INDI::Telescope::updateProperties();
	
	return rc;
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
	
	LOGF_INFO("BresserExosIIDriver::ReadScopeStatus: Pointing to Right Ascension: %f Declination :%f...",currentCoordinates.RightAscension,currentCoordinates.Declination);
	
	NewRaDec(currentCoordinates.RightAscension, currentCoordinates.Declination);
	
	return true;
}

bool BresserExosIIDriver::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
	return false;
}

bool BresserExosIIDriver::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
	return false;
}

bool BresserExosIIDriver::Park()
{
	mMountControl.ParkPosition();
	
	return false;
}
			
bool BresserExosIIDriver::UnPark()
{
	return false;
}

bool BresserExosIIDriver::Sync(double ra, double dec)
{
	return false;
}
			
bool BresserExosIIDriver::Goto(double ra, double dec)
{
	mMountControl.GoTo((float)ra,(float)dec);		
	
	return true;
}
			
bool BresserExosIIDriver::Abort()
{
	mMountControl.StopMotion();
	return true;
}

bool BresserExosIIDriver::updateTime(ln_date *utc, double utc_offset)
{
	
	
	return false;
}
			
bool BresserExosIIDriver::updateLocation(double latitude, double longitude, double elevation)
{
	LOGF_INFO("Location updated: Longitude (%g) Latitude (%g)", longitude, latitude);
	
	mMountControl.SetSiteLocation((float)latitude,(float) longitude);
	
	
	return false;
}
