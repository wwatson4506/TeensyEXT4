/*
  TeensyEXT4 datalogger
 
 This example code is in the public domain.
*/

#include <ext4FS.h>

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
// Four physical drives with four partitions each are supported.
// The built in SD card device is uses sdd1 to sdd3 exclusively.
// Note: lwext4 only supports four mounted partitions total
//       at any one time right now.
// TODO: Explore options to increase this to 16 partitions.
//**********************************************************************
//**********************************************************************
// WARNING: ext4 MUST be cleanly un-mounted to avoid data loss.
//          'myext4fs1.lwext_umount(sdxx);'
//**********************************************************************

//**********************************************************************
// Setup four instances of ext4FS (four mountable partittions).
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

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect.
  }

  Serial.printf("%cTeensy lwext datalogger\n\n",12);
  Serial.println("Initializing ext4FS ...");
  Serial.println("Please wait...");

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
  myext4fs1.init_block_device(sd, 3) == EOK ?
  Serial.printf("SD card is inserted...sd = 0x%x\n",sd) :
  Serial.printf("SD card is NOT inserted...\n");

  if(!myext4fs1.begin(sdxx)) {
    Serial.printf("myext4fs1.begin(%s) Failed: Drive plugged in?\n",mpName[sdxx]);
	while(1); // Give up !!!
  } else {
    Serial.printf("myext4fs.begin(%s): passed...\n",mpName[sdxx]);
  }

  Serial.printf("Volume Name: %s\n",myext4fs1.getVolumeLabel());
  Serial.println("initialization done.");

  Serial.println("logging data...");
  logData();
  Serial.println("Finished logging data...");

  Serial.printf("Unmounting device %s...\n",mpName[sdxx]);
  myext4fs1.lwext_umount(sdxx);
}

void logData(void) {
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }

  // open the file.
  File dataFile = EXT.open("datalog.txt", FILE_WRITE);
  // if the file is available, write to it: 100 Strings...
  if (dataFile) {
	for(int i=0; i <= 100; i++) {
		dataFile.println(dataString);
		// print to the serial port too:
		Serial.println(dataString);
    }
  } else {
    // if the file isn't open, pop up an error:
    Serial.println("error opening datalog.txt");
  }
  dataFile.close();
}

void loop() {
}

