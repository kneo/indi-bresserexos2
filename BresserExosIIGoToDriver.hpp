/*
 * BresserExosIIGoToDriver.hpp
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
 
#ifndef _BRESSEREXOSIIGOTODRIVER_H_INCLUDED_
#define _BRESSEREXOSIIGOTODRIVER_H_INCLUDED_

#include <cstdint>
#include <cmath>
#include <memory>

#include <libindi/indicom.h>
#include <libindi/inditelescope.h>
#include <libindi/indilogger.h>

#include "IndiSerialWrapper.hpp"
#include "ExosIIMountControl.hpp"

#include "Config.hpp"

namespace GoToDriver
{
	class BresserExosIIDriver : public INDI::Telescope
	{
		public:
			BresserExosIIDriver();
			
			virtual ~BresserExosIIDriver();
			
			virtual bool initProperties();
			
			virtual bool updateProperties();
			
			virtual bool Connect();
			
			virtual bool Handshake();

			virtual bool Disconnect();
			
			virtual const char* getDefaultName();
			
			virtual bool ReadScopeStatus();
					
			virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n);

			virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n);
			
			virtual bool Park();
			
			virtual bool UnPark();

			virtual bool Sync(double ra, double dec);
			
			virtual bool Goto(double, double);
			
			virtual bool Abort();
			
			virtual bool SetTrackingEnabled(bool enabled);

			virtual bool updateTime(ln_date *utc, double utc_offset);
			
			virtual bool updateLocation(double latitude, double longitude, double elevation);
			
		private:
			//TODO: serial interface,
			IndiSerialWrapper mInterfaceWrapper;
			
			//TODO: mount control,
			TelescopeMountControl::ExosIIMountControl<IndiSerialWrapper> mMountControl;
			
			unsigned int DBG_SCOPE;
	};
}

#endif
