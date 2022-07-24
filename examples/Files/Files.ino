/*
  lwext4 basic file example
 
 This example code is in the public domain.
*/
#include <ext4FS.h>


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
#define sdxx sda1

File myFile;

void setup()
{
 //UNCOMMENT THESE TWO LINES FOR TEENSY AUDIO BOARD:
 //SPI.setMOSI(7);  // Audio shield has MOSI on pin 7
 //SPI.setSCK(14);  // Audio shield has SCK on pin 14
  
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect.
  }


  Serial.printf("%cTeensy lwext files testing\n\n",12);
  Serial.println("Initializing LWextFS ...");

  myusb.begin();

//ext4_dmask_set(DEBUG_ALL);

  Serial.println("Initializing all availble lwext devices.\n");
  Serial.println("Please Wait...\n");
  int devcount = myext4fs.lwext_init_devices();  
  Serial.print(devcount,DEC);
  Serial.println(" lwext devices found.\n\n");

  if(!myext4fs1.begin(sdxx)) { // Change this to sdd1 for SD card.
    Serial.printf("myext4fs.begin(sdxx) Failed: Drive plugged in?\n");
	while(1); // Give up !!!
  } else {
    Serial.printf("myext4fs.begin(sdxx): passed...\n");
  }

  Serial.printf("Volume Name: %s\n",myext4fs1.getMediaName());
  if (EXT.exists("example.txt")) {
    Serial.println("example.txt exists.");
  }
  else {
    Serial.println("example.txt doesn't exist.");
  }

  // open a new file and immediately close it:
  Serial.println("Creating example.txt...");
  myFile = EXT.open("example.txt", FILE_WRITE);
  myFile.close();

  // Check to see if the file exists: 
  if (EXT.exists("example.txt")) {
    Serial.println("example.txt exists.");
  }
  else {
    Serial.println("example.txt doesn't exist.");  
  }

  // delete the file:
  Serial.println("Removing example.txt...");
  EXT.remove("example.txt");

  if (EXT.exists("example.txt")){ 
    Serial.println("example.txt exists.");
  }
  else {
    Serial.println("example.txt doesn't exist.");  
  }
  Serial.println("Unmounting sdxx...");
  myext4fs1.lwext_umount(sdxx);
}

void loop()
{
  // nothing happens after setup finishes.
}



