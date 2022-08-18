// Format an block device to ext4.

#include "ext4FS.h"

USBHost myusb;

// Setup USBHost_t36 and as many HUB ports as needed.
USBHub hub5(myusb);
USBHub hub6(myusb);
USBHub hub7(myusb);
USBHub hub8(myusb);

// Instances for the number of USB drives you are using.
USBDrive extDrive1(myusb);
USBDrive extDrive2(myusb);
USBDrive extDrive3(myusb);

USBDrive *drive_list[] = {&extDrive1, &extDrive2, &extDrive3};

// EXT usage with an SDIO card.
#define SD_CONFIG SdioConfig(FIFO_SDIO)
SdCardFactory cardFactory;
SdCard *sd = cardFactory.newCard(SD_CONFIG);

//**********************************************************************
// Setup four instances of LWextFS (four mountable partittions).
//**********************************************************************
ext4FS myext4fs1;
ext4FS myext4fs2;
ext4FS myext4fs3;
ext4FS myext4fs4;

#define sda1 0	// First partition on first USB device.
#define sda2 1	// Second partition on first USB device.
#define sdb1 4	// First partition on second USB device.
#define sdc1 8	// First partition on third USB device.
#define sdd1 12  // First partition on SD device

// Set this to one of the above devices.
#define sdxx sda1

// Grab a pointer to the mount list.
bd_mounts_t *ml = myext4fs1.get_mount_list();

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
  Serial.println("Initializing ext4FS ...");
  Serial.println("Initializing all availble lwext4 devices.\n");
  Serial.println("Please Wait...\n");

  myusb.begin();
  delay(3000);
  myusb.Task();

  for (uint16_t drive_index = 0; drive_index < (sizeof(drive_list)/sizeof(drive_list[0])); drive_index++) {
    USBDrive *pdrive = drive_list[drive_index];
    if (*pdrive) {
      if (!pdrive->filesystemsStarted()) {
        pdrive->startFilesystems();
      }
      myext4fs1.init_block_device(pdrive, drive_index);  
    }
  }
  // Init SD card (Block device 3) fixed.
  if(myext4fs1.init_block_device(sd, 3) == EOK) {  
    Serial.printf("SD card is inserted...\n");
  } else {
    Serial.printf("SD card is NOT inserted...\n");
  }

  Serial.print("\nWARNING: This will destroy any data on ---> ");
  Serial.println(mpName[sdxx]);
  waitforInput();

  int result = myext4fs1.lwext_mkfs(&ml[sdxx].partbdev, "32GUSBEXT4");
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
