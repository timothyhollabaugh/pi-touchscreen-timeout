# pi-touchscreen-timeout
Leaving the backlight on the Official Raspberry Pi touchscreen can quickly wear it out. If you have a use that requires the pi to be on all the time, but does not require the display on all the time, then turning off the backlight while not in use can dramatically increase the life of the backlight.

pi-touchscreen-timeout will transparently turn off the display backlight after there have been no touches fo a specifed timeout, independent of anything using the display at the moment. It will then turn the touchscreen back on when it is touched.

**Note:** This does not stop the touch event from getting to whatever is running on the display. Whatever is running will still recive a touch event, even if the display is off

The default timeout is ten seconds. It is defined in the top of timeout.c if you want to change it.

It uses the linux event device `/dev/input/event0` to recive events from the touchscreen, and `/sys/class/backlight/rpi-backlight/bl_power` to turn the backlight on and off

#Installation

Clone the repository:
`git clone https://github.com/chickenchuck040/pi-touchscreen-timeout.git`

Run it!
```
cd pi-touchscreen-timeout
sudo ./timeout
```

**Note:** It must be run as root or with `sudo` to be able to access the backlight.

It is recommended to make it run at startup, for example putting a line in `/etc/rc.local`


