# TeensyEXT4

This is the fourth version of lwext4 adapted for use with the Teensy T3.6/T4.x. 

This is the start of being able to use lwext4 with the T36/T40/T41. This version supports MSC USB drives and SD cards (Teensy built in only). It has only been tested with Arduino 1.8.19 and Teenyduino 1.57/1.58B2. It can mount an ext4 formatted USB drive or SD card.

You will need Arduino 1.8.19 or the Arduino IDE and Teensyduino 1.60B1 to 1.60B5.
Teensyduino can be downloaded from here:
https://www.pjrc.com/teensy/td_download.html

#### UPDATES:
*  This version eliminates the need for the LWextFS class. Examples updated.
*  FS abstraction layer now supported. 
*  Formatting USB and SD devices now Supported.
*  Copying files.
*  VGA_4bit_T4 now supported.
  
You will need a USB drive or SD card formatted as ext4 with a volume label to identify the drive. Compile any of the TeensyEXT4 examples and upload to the Teensy.

There is a config file 'ext4_config.h' in the 'src' directory. The default settings are the best ones so far for the Teensy. Be aware that journaling is not working right now so that has been disabled.

For reference:

https://github.com/gkostka/lwext4 Original Version.

https://github.com/gkostka/stm32f429disco-lwext4 An example using USB drive as a block device.

https://github.com/autoas/as/tree/master/com/as.infrastructure/system/fs  

#### Example Sketches:
- ext4Usage.ino This sketch contains information on the general usage of TeensyEXT4 and is similar to LittleFS_Usage and SdFat_Usage.
- ext4DeviceInfo.ino Gives various stats about a mounted block device and it's partitions.
- ListFiles.ino Gives a directory listing.
- Files.ino Tests file existance.
- Datalogger.ino Logs ADC data to a block device.
- Dump.ino Dumps the data from Datalogger.ino.
- ReadWrite.ino Demonstrates reading and writing test strings to a block device (append mode).
- wavePlayerExt4.ino A sketch that plays wav files.
- copyFilesUSB.ino Copy files between USB and SD devices. 2 USB and 1 SD device supported.
- ext4Formatter.ino Formats a USB or SD device to ext4. (Very Slow on big drives)

#### WARNING: Fromatting an SD device works good but formatting a USB device is very SLOW! You might as well take a trip to the Bahamas and when you get back it might be done:)
Have not figured out why yet.

These are the example files so far.
At this time it parallels SD and LittleFS fairly close.

#### TODO:
- Set and get user and group permissions.
- Set and get user and group ID's.
- Implement chdir().

There is a lot of capability in lwext4 that has not yet been added. Some of it may not be practical to use in this application.

#### Limitations:
 Four physical drives with four partitions each are supported.
 The built in SD card device is uses sdd1 to sdd3 exclusively.
 Note: lwext4 only supports four mounted partitions total
       at any one time right now.
#### TODO: Explore options to increase this to 16 partitions.
 
 ## WARNING: ext4 MUST be cleanly un-mounted to avoid data loss.
 ##            'myext4fs1.lwext_umount(sdxx);'
#### Fixes:
 - Added missing ext4_unregister() to lwext4_umount().

This is still very much work in progress # WIP

More to come...
