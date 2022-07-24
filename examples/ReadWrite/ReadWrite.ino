/*
  lwext4 read/write

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
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect.
  }

  Serial.printf("%cTeensy lwext read/write test\n\n",12);
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
  Serial.println("initialization done.");
  
  // open the file. 
  myFile = EXT.open("test.txt", FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
	// close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  
  // re-open the file for reading:
  myFile = EXT.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
    	Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  Serial.println("Unmounting sdxx...");
  myext4fs1.lwext_umount(sdxx);
}

void loop()
{
	// nothing happens after setup
}


