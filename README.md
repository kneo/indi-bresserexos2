# Exos II GoTo Telescope Mount Driver for libindi

## Disclaimer
You get the driver free of charge and, also may modify it to your needs.
However the software is distributed AS IS.
I do not take responsibility for any Damages, caused by the use this software, to YOUR equipment or health!

## Introduction
This is a basic driver for the Bresser Exos II GoTo telescope mount controller, allowing the connection to Indi clients/software.
The driver is intended for remote control on a Raspberry Pi, running Astroberry with libindi, but may run on any Indi running platform.
Its current state is experimental, but hopefully gradually improves. 
Since its the initial release, feedback for improvement is appreciated.
If your have an improvement or feature to add, please fell free to write a mail, a ticket in the issues section or a pull request.

## Requirements
- Raspberry Pi with Astroberry with Libindi 1.8.7 (https://www.astroberry.io/, https://www.indilib.org/)
- latest Version of cmake installed
- Astronomy Software (KStars, for Windows download see: https://edu.kde.org/kstars/#download, or use the package manager of your linux distribution.)
- Firmware Version 2.3 installed on the Handbox (https://www.bresser.de/Astronomie/Zubehoer/Motoren-Steuerungen/BRESSER-Computer-Kabel-zur-Fernsteuerung-von-MCX-Goto-Teleskopen-und-EXOS-II-EQ-Goto-Montierungen.html, under Manual)
- A COM Port or working USB to Serial Adapter (any device supporting the change of Baud Rates will do!)
- The Bresser Serial Adapter for the Handbox (https://www.bresser.de/Astronomie/Zubehoer/Motoren-Steuerungen/BRESSER-Computer-Kabel-zur-Fernsteuerung-von-MCX-Goto-Teleskopen-und-EXOS-II-EQ-Goto-Montierungen.html)
- The Bresser Exos II GoTo Mount (or the Update Kit) (https://www.bresser.de/Astronomie/BRESSER-Messier-EXOS-2-EQ-GoTo-Montierung.html, https://www.bresser.de/Astronomie/Zubehoer/Motoren-Steuerungen/BRESSER-StarTracker-GoTo-Kit.html)

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
10. Remain here do not go Connect to the desktop rather Go To INDI Webmanager:
	- Click the little scope on the side bar
11. Look throught the Driver list for "Exos II GoTo"
	- If it appears, the installation is completed.
	
## Connection Configuration

### KStars
I happend to use kstars for configuration, it provides a wizard to set up the connection.
Enter the HTTP Adress of your raspberry pi, and KStars will setup a profile for you.
You need to enter "Exos II GoTo" manually in the Mount Driver Combobox, since kstars does not list this device (at least not in my case).

### Stellarium
You can also use Stellarium, but you need to set up an Indi instead of the Ascom device (assuming you previously used this).

## Important Note for Observation Setup
It is ** important ** that you put the scope in the Home position, Polar and Star Align in accordance to the manual.
** Its vital in order to avoid damage to your Equipment. The Driver does not HANDLE this for your! **