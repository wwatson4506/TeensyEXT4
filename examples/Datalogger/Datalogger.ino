/*
  lwext4 datalogger
 
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

  Serial.printf("%cTeensy lwext datalogger\n\n",12);
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

  Serial.println("logging data...");
  logData();
  Serial.println("Finished logging data...");

  Serial.println("Unmounting device...");
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

