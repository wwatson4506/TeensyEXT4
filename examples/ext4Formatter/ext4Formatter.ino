// Format an block device to ext4.

#include "ext4FS.h"

extern USBHost myusb;

LWextFS myext4fs;
LWextFS myext4fs1;
LWextFS myext4fs2;
LWextFS myext4fs3;
LWextFS myext4fs4;

#define sda1 0	// First partition on first USB device.
#define sda2 1	// Second partition on first USB device.
#define sdb1 4	// First partition on second USB device.
#define sdc1 8	// First partition on third USB device.
#define sdd1 12  // First partition on SD device

// Set this to one of the above devices.
#define sdxx sdd1

// Grab a pointer to the mount list.
bd_mounts_t *ml = get_mount_list();

void setup() {
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) { ; // wait for Arduino Serial Monitor
  }
   while (!Serial) {
    yield(); // wait for serial port to connect.
  }
  if(CrashReport)
	Serial.print(CrashReport);

//ext4_dmask_set(DEBUG_ALL);

  Serial.printf("%cTeensy ext4 formatter\n\n",12);
  Serial.println("Initializing LWextFS ...");

  myusb.begin();

  Serial.println("Initializing device sdxx.\n");
  myext4fs.init_block_device(sdxx);  
  
  Serial.print("\nWARNING: This will destroy any data on ---> ");
  Serial.println(mpName[sdxx]);
  waitforInput();

  int result = myext4fs1.lwext_mkfs(&ml[sdxx].partbdev, "32GSDEXT4");
  if(result != EOK)
    Serial.printf("ext4 format failed: %d\n", result);
  else  
    Serial.println("ext4 format passed...");

  Serial.println("Done...");

}

void loop() {
}

void waitforInput()
{
  Serial.println("Press any key to continue\n");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
