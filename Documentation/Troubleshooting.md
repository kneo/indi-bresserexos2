# Troubleshooting

## Safety Precautions:
- Set up the mount in accordance to the manual
    - make especially sure to align the triangle arrow marks to face each other, its the "home" position of the mount and assumes this position always prior to starting.
- Always make sure you are able power off the device without touching it, to avoid injuries
    - pull the power plug rather than flip the power switch

## USB Troubleshooting

### Wrong device node
The driver is not able to determine whether its connected to the correct serial device prior to communication being successfully established.
However the driver issues a log message to the user, if it is not able to communicate with the mount. Furthermore the device enum on linux devices may shuffle the device nodes around.
To fix this you can order udev to create a custom symlink to the device node by createing a new rule file.
Use `lsusb` do retrieve the identifier of your usb to serial adapter:

![Get product and vendor id](get-usb-product-vendor-id.png?raw=true)

> sudo nano /etc/udev/rules.d/10-local.rules

add:
<code>ACTION=="add", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", SYMLINK+="my_uart"</code>

change `ATTRS` values accordingly. Change the `SYMLINK``value to a name fitting your needs.

Use `CTRL`+`O` to save the file and `CTRL`+`x` to close the editor.

After unplugging and plugging in the adapter a new device node in `dev` is created with the name chosen.

You can change the serial name in the EKO configuration dialog.

**Caveate:** Since some manufacturers do not bother to name their devices properly, and do not request a proper vendor id this solution only works as long as you have not several devices sharing the same vendor and product ids. 

### Permission problems
Some linux distributions block users from using serial adapters which are not in a specific group. For astroberry it is called `dialout`.
Use `groups` to find out what groups your user is in:

![dmesg output example](get-groups-list.png?raw=true)

if your user does not have the `dialout` group in this list you may add it using:

> sudo gpasswd --add ${USER} dialout

The device permission issue should now be resolved after the next login.