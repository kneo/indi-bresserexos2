# Exos II GoTo Telescope Mount Driver for libindi

---

## Disclaimer
You get the driver free of charge and, also may modify it to your needs.

However the software is distributed AS IS.

**I'M NOT RESPONSIBLE FOR ANY DAMAGES OR INJURIES, CAUSED BY THIS SOFTWARE!**

---

## Introduction
This is a basic driver for the Bresser Exos II GoTo telescope mount controller, allowing the connection to Indi clients/software.
The driver is intended for remote control on a Raspberry Pi, running Astroberry with libindi, but may run on any Indi running platform.
Its current state is experimental, but hopefully gradually improves.
Since its the initial release, feedback for improvement is appreciated.

If your have an improvements, features to add or a bug to report, please fell free to write a mail, a ticket in the issues section or a pull request.

## About the Mount
The Bresser Exos II GoTo Mount has a relabled JOC SkyViewer Handbox.
It runs the Firmware Version 2.3 distributed by Bresser.
The mount is highly autonomous, in terms motion controls, when initialized properly I did not notice any jams or crashes.
On the serial protocol side however, this device is quite primitive. 
The data exchange is established using a 13 Byte message frame, with a 4 Byte preamble, leaving 1 byte for a command and 8 bytes for command parameter data.
The protocol only accepts, a few commands for goto tracking, parking, motion stop and Location/Time/Date setting.

It seems also to have some "hidden" commands, which do not provoke an error state, but do not have an appearent effect either.
I reverse engineered the the most useful parts of the serial protocol using serial port sniffing tools, developping this driver as a result. 

## Requirements
- Raspberry Pi with Astroberry (AB) with Libindi 1.8.7 (https://www.astroberry.io/, https://www.indilib.org/), In fact: any platform running indi will do, but I developped this with AB in mind,
- latest Version of cmake installed (at least Version 3.13)
- Astronomy Software (KStars, for Windows download see: https://edu.kde.org/kstars/#download, or use the package manager of your linux distribution.)
- A COM Port or working USB to Serial Adapter (any device supporting the change of Baud Rates will do!)
- The Bresser Serial Adapter for the Handbox (https://www.bresser.de/Astronomie/Zubehoer/Motoren-Steuerungen/BRESSER-Computer-Kabel-zur-Fernsteuerung-von-MCX-Goto-Teleskopen-und-EXOS-II-EQ-Goto-Montierungen.html)
- The Bresser Exos II GoTo Mount (or the Upgrade Kit) (https://www.bresser.de/Astronomie/BRESSER-Messier-EXOS-2-EQ-GoTo-Montierung.html, https://www.bresser.de/Astronomie/Zubehoer/Motoren-Steuerungen/BRESSER-StarTracker-GoTo-Kit.html)
- Firmware Version 2.3 installed on the Handbox (https://www.bresser.de/Astronomie/Zubehoer/Motoren-Steuerungen/BRESSER-Computer-Kabel-zur-Fernsteuerung-von-MCX-Goto-Teleskopen-und-EXOS-II-EQ-Goto-Montierungen.html, under Manual)

## Features
- Works with KStars/Stellarium using Indi Connection
- GoTo Coordinates and Track commands (Sidereal Tracking)
- Park and Abort commands
- Sync commands for alignment
- Get/Set Site Location
- Set Date/Time

## Prerequisites
Make sure you have software in this section installed before the build attempt.

### Libindi
Check if you have the libindi software suite installed:

>``sudo apt-get install libindi1 libindidriver1 libindi-data libindi-dev libindi-plugins libindialignmentdriver1``

This should be done on astroberry, but if you are using another distribution, this may be necessary.

### Development Tools and Build System
Please check if cmake is installed, since its the primary build system and throw in a build-essential for good measure.

>``sudo apt-get install cmake build-essential``

This should already be installed on Astroberry, but may be necessary on a different distribution.

Wait until everything is installed, and continue with building the driver.

## Building and Install the Driver
1. Clone the Repository into a directory of the Pi (eg. your Home directory):
> ``git clone https://github.com/kneo/BresserExosIIDriverForIndi.git``

2. Change Directory to BresserExosIIDriverForIndi:
> ``cd BresserExosIIDriverForIndi``

3. Create a directory "build" in the current directory:
> ``mkdir build``

4. Change to the build directory:
> ``cd build``

5. Run cmake (and wait for finish):
> ``cmake ..``
	- you can use the CMAKE_INSTALL_PREFIX to adjust the install location of the driver binary if necessary.
	- XML_INSTALL_DIR to adjust the location of the xml file for indi if necessary.

6. Run build process (and wait for the conclusion):
> ``cmake --build .``

7. Install the driver:
> ``sudo make install``

8. Restart the indi Service or simply restart the Pi
	- This is necessary for the driver to be visible in the Web Manager

9. Go To the Astroberry Manager Web Page:
	- http://ip-of-your-pi/desktop

10. Remain here do not "Connect" to the desktop, rather Go To INDI Webmanager:
	- Click the little scope on the side bar

11. Look throught the Driver list for "Exos II GoTo"
	- If it appears, the installation is completed.

## Important Note before Further Setup or Observation
It is **important** that you put the scope in the Home position, Polar and Star Align in accordance to the manual.

**Its vital in order to avoid damage to your Equipment. This Driver does not HANDLE this for your!**

## Client Software Configuration
Hock up your Serial Cable to the Handbox and the USB-To-Serial Adapter and connect the USB-To-Serial Adapter to the Pi/Computer.

Enter the Command:
> ``ls -l /dev/ttyUSB*``

To see if a Serial device has appeared.
This device name is required to configure the driver.
If not, use:

> ``dmesg``

to see if your adapter has a different name.

### KStars
I happend to use kstars for configuration, it provides a wizard to set up the connection.
In this Wizzard enter the Address of your Astroberry Pi, and KStars will setup a profile for you.
You need to enter "Exos II GoTo" manually in the Mount Driver Combobox, since kstars does not list this device (at least not in my case).
Usually everything else falls into places here. When connecting to the Indiservice in kstars, it should already have selected your serial port.
But depending on how much equipment you have hocked up, you may have to choose the right port for your mount profile.

### Stellarium
You can also use Stellarium, but you need to set up an Indi instead of the Ascom device (assuming you previously used this).
The Telescope Control plug in is quite simple. You just need to select the Indi connector, and enter the IP-Address of your Rasperry PI in the "Indi Settings" Groupbox.
Click "Refresh Devices" to get a list with available devices in the Combo-Box below. Select the "Bresser Exos II GoTo Driver (...)".

In Stellarium commands to for GoTo are supported, no parking or stopping, but it updates the telescope pointing coordinates in the Sky View.

### Test Utiltiy
The repository also contains a little test utility accessing the serial device directly.
It can be used to try out the driver functionallty without a lot of configuration, or without messing up existing configurations.
Building the driver should also build the tool, to use it just call:

> ``./BresserExosIIGoToDriverForIndiTest /dev/ttyUSB0``

*(your serial device may be different)*

It requires exclusive access to the interface, so make sure nothing else is using it.
You will be introduced to a simple menu system allowing to send the commands implemented in the serial protocol.

### Testing your Connection
Once you have a set up profile in e.g. KStars issue a "park" command to see if everything, is working. 
The hand controller should sound a single beep from the handbox.
With this you can start doing your observation.

## Known Issues
- Tracking modes can not be set, only Sidereal Tracking is working right now.
- KStars only updates location but not the time.
- Sync only works when tracking an object.
- Newer versions of indi (Version 1.8.8) may break the build, since the driver interface has changed.

## Thanks
- Thanks to spitzbube for his effort in reverse engineering the handbox (https://github.com/Spitzbube/EXOS-2_GoTo_HandController).