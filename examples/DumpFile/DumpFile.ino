/*
  lwext4 file dump
 
 This example code is in the public domain.
*/

#include <ext4FS.h>

extern USBHost myusb;

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
#define sdxx sda1

void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect.
  }


  Serial.printf("%cTeensy lwext dump file\n\n",12);
  Serial.println("Initializing LWextFS ...");

  myusb.begin();

//ext4_dmask_set(DEBUG_ALL);

  if(!myext4fs1.begin(sdxx)) { // Change this to sdd1 for SD card.
    Serial.printf("myext4fs.begin(sdxx) Failed: Drive plugged in?\n");
	while(1); // Give up !!!
  } else {
    Serial.printf("myext4fs.begin(sdxx): passed...\n");
  }

  Serial.printf("Volume Name: %s\n",myext4fs1.getMediaName());
  Serial.println("initialization done.");

  // open the file.
  File dataFile = EXT.open("datalog.txt");
  Serial.print("File Size: ");
  Serial.println(dataFile.size());
  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 
  Serial.println("Finished Data Dump...");

  Serial.println("Unmounting sdxx...");
  myext4fs1.lwext_umount(sdxx);
}

void loop()
{
}

