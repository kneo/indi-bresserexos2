# Application Setup

## Basic Hardware/Software Setup
Hock up your Serial Cable to the Handbox and the USB-To-Serial Adapter and connect the USB-To-Serial Adapter to the Pi/Computer.

Enter the Command:
> ``ls -l /dev/ttyUSB*``

To see if a Serial device has appeared.
This device name is required to configure the driver.
If not, use:

> ``dmesg``

to see if your adapter has appeared under a different name:

![determine usb device node name](dmesg-usb-device-name-determination.png?raw=true)

**Note:** Device enumeration in linux is sometimes arbitrary, and if multiple USB-to-Serial devices exist, those may be shuffled around. Keep in mind, that the driver does not know what device is connected using which device node.
See [Troubleshooting](Troubleshooting.md) if this problem occurs.

### Start and Setup Astroberry Indi Server
Astroberry is the hub of your setup so power up you pi and wait until its booted.

To start open a webbrowser to and enter the IP-Address or DNS-Name of your raspberry pi running astroberry:
> http://ip.of.your.pi/
or
> http://astroberry/

On the start up page, click simply click "Start".
![start astroberry](start-up-astroberry-1.png?raw=true)

Loading the next page may take some time.

Initially you need to create a new setup profile in astroberry server to do this click the tiny telescope icon in the small side bar on the left.

![locate scope icon](start-up-astroberry-2.png?raw=true)

The INDI Web Manager panel appears.
Set an name in the "New Profile" text box and select your devices in the "Local Drivers" combobox as shown in the example.

![Set up profile](start-up-astroberry-3.png?raw=true)

press the "plus" button to add the profile to your configurations.

After selecting the configuration from the "Equipment Profile" combo box click the start button  to actually launch the server.

![Server launched](start-up-astroberry-4.png?raw=true)
It should state "Server is online" in green, and the "Gear" in the "Stop" button should be spinning.

On the next setup of your equipment it suffices to select the "Equipment Profile" and hit "Start" again to get up and running.

This concludes the INDI Setup part.

### Choose Prefered Operation Mode
You can either choose to use VNC connection to run your Astronomy Software on the Pi directly or use a dedicated client to connect to the INDI Server. Both ways work analogous.

### KStars
Start KStars and perform the Initial Wizard, if not already concluded priorly.

When done open the "Ekos" Window by clicking the tiny dome Icon in tool bar.

![Open Ekos](setup-kstars-1.png?raw=true)

This process may also nag you with a "Ekos Profile Wizard". You can simply close it.

Initially it should only contain a "Simulators" profile, ignor it and create a new one, by clicking at the "plus" icon.

![Open Ekos](setup-kstars-2.png?raw=true)

The window poping up queries some information from your. To set a profile on a choose a name and enter the IP address of your Raspberry Pi running astroberry (if you use a VNC connection to the PI you can go with "localhost").

The Kstars Ekos dialog requires you to select at least a mount and a ccd elsewise it refuses to start.

![create profile](setup-kstars-3.png?raw=true)

You have to enter "BRESSER Messier EXOS-2 EQ GoTo" (simply copy and paste) manually in the Mount Driver Combobox, since kstars does not list this remote devices (at least not in my case).

If you do not have as CCD, go with "CCD Simulator" which should be selectable from the combobox.

Click save to complete the setup.

After finishing select your newly created profile from the "Profile" combobox and click the "play arrow"

![Launch profile](setup-kstars-4.png?raw=true)

The "INDI Control Panel" will pop up, signaling everything is running. 
You should be able to "unpark" your telescope from the Star panel now and issue pointing commands using the context menu.

![Launch profile](setup-kstars-5.png?raw=true)

### Stellarium
You can also use Stellarium, but you need to set up an Indi instead of the Ascom device (assuming you previously used it).
The Telescope Control plug in is quite simple. You just need to select the Indi connector, and enter the IP-Address of your Rasperry PI in the "Indi Settings" Groupbox.
Click "Refresh Devices" to get a list with available devices in the Combo-Box below. Select the "BRESSER Messier EXOS-2 EQ GoTo".

In Stellarium commands to for GoTo are supported, no parking or stopping, but it updates the telescope pointing coordinates in the Sky View.
