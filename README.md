# TeensyEXT4

This is a working version of lwext4 adapted for use with the Teensy T3.6/T4.x. 

This is the start of being able to use lwext4 with the T36/T40/T41 and so far MSC USB drives. It has only been tested with Arduino 1.8.19 and Teenyduino 1.57B2. It will mount an ext4 formatted USB drive through USBHost_t36 which is included in TD1.57B2.

You will need Arduino 1.8.19 and:

https://forum.pjrc.com/threads/70409-Teensyduino-1-57-Beta-2

***************************************
* UPDATE: Added support for SD cards. *
***************************************
In setup() you will find these lines. only one can be uncommented at a time.

  parent_blockdev = ext4_usb_bd_get();
//  Uncomment the line below and comment line above 
//  to test SDIO card.  
//  parent_blockdev = ext4_sd_bd_get();


You will need a USB drive formatted as ext4 with a volume label to identify the drive. Then you will need to mount it in Linux and use a filemanager to create a folder named 'mp'. This is the mount point (device file) for TeensyEXT4 access to the USB device. Compile the the TeensyEXT4test.ino example and upload to the Teensy. It should then mount the USB drive and give info on the MBR and the testing results.

At the begining of the test sketch you will see:

// ******** These are changable for testing purposes *****************

/**@brief   Read-write size*/
static int rw_szie = 32 * 1024;

/**@brief   Read-write size*/
static int rw_count = 1000;

/**@brief   Directory test count*/
static int dir_cnt = 0;

/**@brief   Cleanup after test.*/
static bool cleanup_flag = false;

/**@brief   Block device stats.*/
static bool bstat = false;

/**@brief   Superblock stats.*/
static bool sbstat = false;

/**@brief   Verbose mode*/
static bool verbose = false;

//********************************************************************

You can set these for different levels of testing and debugging.

There is a config file 'ext4_config.h' in the 'src' directory. The default settings are the best ones so far for the Teensy. Be aware that journaling is not working right now so that has been disabled.

For reference:

https://github.com/gkostka/lwext4 Original Version.

https://github.com/gkostka/stm32f429disco-lwext4 An example using USB drive as a block device.

https://github.com/autoas/as/tree/master/com/as.infrastructure/system/fs  

Example of mouting multiple partitions (RTOS).

Key Files:

- TeensyEXT4test.ino example file
- usb_bd.c and usb_bd.h the main USB drive access files for TeensyEXT4

These are the files that interface the Teensy to lwext4.

TODO:

- Add access to ext4 formatted SD cards.
- Create more test sketches for testing.
- Restructure for proper integration with FS.

This is still very much work in progress # WIP

More to come...
