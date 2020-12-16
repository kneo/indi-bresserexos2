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
	return false;
}

bool BresserExosIIDriver::Connect()
{
	bool rc = INDI::Telescope::Connect();
	
	LOGF_INFO("BresserExosIIDriver::Connect: Initializing ExosII GoTo on %d...",PortFD);
	
	if(rc==true)
	{
		mInterfaceWrapper.SetFD(PortFD);
		
		mMountControl.Start();
		return true;
	}
	
	return false;
}

bool BresserExosIIDriver::Handshake()
{
	bool rc = INDI::Telescope::Handshake();

	if(rc==true)
	{
		//mInterfaceWrapper.SetFD(-1);
		

		return true;
	}

	return rc;
}

bool BresserExosIIDriver::Disconnect()
{
	bool rc = INDI::Telescope::Disconnect();
	
	if(rc==true)
	{
		mMountControl.Stop();
		mInterfaceWrapper.SetFD(-1);
		return true;
	}
	
	return false;
}

const char* BresserExosIIDriver::getDefaultName()
{
	return "Bresser Exos II GoTo Driver (for Firmware V2.3)";
}

bool BresserExosIIDriver::ReadScopeStatus()
{
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
			
bool BresserExosIIDriver::Goto(double, double)
{
	return false;
}
			
bool BresserExosIIDriver::Abort()
{
	return false;
}

bool BresserExosIIDriver::updateTime(ln_date *utc, double utc_offset)
{
	
	
	return false;
}
			
bool BresserExosIIDriver::updateLocation(double latitude, double longitude, double elevation)
{
	LOGF_INFO("Location updated: Longitude (%g) Latitude (%g)", longitude, latitude);
	
	
	
	return false;
}
