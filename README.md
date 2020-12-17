# Exos II GoTo Telescope Mount Driver for libindi

## Disclaimer
You get the driver free of charge and, also may modify it to your needs.

However the software is distributed AS IS.

**I'M NOT RESPONSIBLE FOR ANY DAMAGES OR INJURIES, CAUSED BY THIS SOFTWARE!**

## Introduction
This is a basic driver for the Bresser Exos II GoTo telescope mount controller, allowing the connection to Indi clients/software.
The driver is intended for remote control on a Raspberry Pi, running Astroberry with libindi, but may run on any Indi running platform.
Its current state is experimental, but hopefully gradually improves.
Since its the initial release, feedback for improvement is appreciated.

If your have an improvements or features to add, please fell free to write a mail, a ticket in the issues section or a pull request.

## About the Mount
The Bresser Exos II GoTo Mount has a relabled JOC SkyViewer Handbox. It runs the Firmware Version 2.3 distributed by Bresser.
The mount is highly autonomous, in terms motion controls, when initialized properly I did not notice any jams or crashes.
On the serial protocol side however, this device is quite primitive. 
It only accepts, a few commands for goto tracking, parking, motion stop and Location/Time/Date setting commands.
I reverse engineered the serial protocol using serial port sniffing tools, developping this driver as a result. 

## Requirements
- Raspberry Pi with Astroberry with Libindi 1.8.7 (https://www.astroberry.io/, https://www.indilib.org/)
- latest Version of cmake installed (at least Version 3.13)
- Astronomy Software (KStars, for Windows download see: https://edu.kde.org/kstars/#download, or use the package manager of your linux distribution.)
- A COM Port or working USB to Serial Adapter (any device supporting the change of Baud Rates will do!)
- The Bresser Serial Adapter for the Handbox (https://www.bresser.de/Astronomie/Zubehoer/Motoren-Steuerungen/BRESSER-Computer-Kabel-zur-Fernsteuerung-von-MCX-Goto-Teleskopen-und-EXOS-II-EQ-Goto-Montierungen.html)
- The Bresser Exos II GoTo Mount (or the Update Kit) (https://www.bresser.de/Astronomie/BRESSER-Messier-EXOS-2-EQ-GoTo-Montierung.html, https://www.bresser.de/Astronomie/Zubehoer/Motoren-Steuerungen/BRESSER-StarTracker-GoTo-Kit.html)
- Firmware Version 2.3 installed on the Handbox (https://www.bresser.de/Astronomie/Zubehoer/Motoren-Steuerungen/BRESSER-Computer-Kabel-zur-Fernsteuerung-von-MCX-Goto-Teleskopen-und-EXOS-II-EQ-Goto-Montierungen.html, under Manual)

## Features
- Works with KStars/Stellarium using Indi Connection
- GoTo Coordinates and Track commands (Sidereal Tracking)
- Park and Abort commands
- Set Site Location and Date/Time

## Install
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

6. Run build process:
> ``cmake --build .``

7. Install the driver:
> ``sudo make install``

8. Restart the Service or the Pi
	- This was necessary for the driver to be visible in the Web Manager

9. Go To the Astroberry Manager Web Page:
	- http://ip-of-your-pi/desktop

10. Remain here do not "Connect" to the desktop rather Go To INDI Webmanager:
	- Click the little scope on the side bar

11. Look throught the Driver list for "Exos II GoTo"
	- If it appears, the installation is completed.

## Important Note before Further Setup or Observation
It is **important** that you put the scope in the Home position, Polar and Star Align in accordance to the manual.

**Its vital in order to avoid damage to your Equipment. The Driver does not HANDLE this for your!**

## Connection Configuration
Hock up your Serial Cable to the Handbox and the USB-To-Serial Adapter and connect the USB-To-Serial Adapter to the Pi/Computer.
Enter the Command:
> ``ls -l /dev/ttyUSB*``
To see if a Serial device has appeared.
If not, use:
> ``dmesg``
to see if the adapter has a different name.

### KStars
I happend to use kstars for configuration, it provides a wizard to set up the connection.
Enter the HTTP Adress of your raspberry pi, and KStars will setup a profile for you.
You need to enter "Exos II GoTo" manually in the Mount Driver Combobox, since kstars does not list this device (at least not in my case).

### Stellarium
You can also use Stellarium, but you need to set up an Indi instead of the Ascom device (assuming you previously used this).

### Testing your Connection
Once you have a set up profile in e.g. KStars issue a "park" command to see if everything, is working. 
The hand controller should sound a single beep from the handbox.
With this you can start doing your observation.

## Known Issues
- When connected, the Mount controller does not send status reports, automatically. You need to manually issue a "stop"/"park" command, for now!
- The build system may not be configurated well enough, for wide integration.
- Manual Slewing from a PC does not work.
- Tracking modes can not be set.
- KStars only updates location but not the time.