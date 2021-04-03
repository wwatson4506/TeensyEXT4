# TeensyEXT4
This is a version of lwext4 adapted for use with T3.6/T4.0/T4.1. Very much WIP

This is the start of being able to use lwext4 with the T36/T40/T41 and so far MSC USB drives. It has only been tested with Arduino 1.8.13 and Teenyduino 1.54B7. At this point is mounting a ext4 formatted USB drive through MSC which is included in TD1.54B7. It also is giving stats on the mounted partition.

You will need a USB drive formatted as ext4 with a volume label to identify the drive. Then you will need to mount it in linux and use a filemanager opened with sudo or root permissions. You will need this to create a folder named 'mp' (For a Mount Point). There will also be a Lost and Found folder that should be moved into the mp folder. After that compile the the TeensyEXT4test.ino example and upload to the Teensy. It should then mount the USB drive and give info on the MBR and the mounted drive including the label. After that it's still work in progress.

You will need:

https://forum.pjrc.com/threads/66357-Teensyduino-1-54-Beta-7

For reference:

https://github.com/gkostka/lwext4 Original Version.

https://github.com/gkostka/stm32f429disco-lwext4 An example using USB drive as a block device.

https://github.com/autoas/as/tree/master/com/as.infrastructure/system/fs  Example of mouting multiple partitions (RTOS).

More to come...
