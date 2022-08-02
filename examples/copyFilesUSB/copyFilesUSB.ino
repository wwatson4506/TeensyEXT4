/*
  LWext4 Multi MSC USB Drive and SD card filecopy testing. 
   
 This example shows how use the mscFS and SD libraries to copy files.
 between USB, SDIO and External SD card devices. It also demonstrates
 hot plugging both USB drives and SD cards. There are two functions that
 do this. They both will try to re-mount the devices if they are not
 mounted.
 	
 Created 07-28-2022
 by Warren Watson
*/

#include "SPI.h"
#include "Arduino.h"
#include "ext4FS.h"

extern USBHost myusb; // Located and defined in 'ext4FS.cpp'.

//**********************************************************************
// Setup four instances of LWextFS (four mountable partittions).
//**********************************************************************
LWextFS myext4fs;
LWextFS myext4fs1;
LWextFS myext4fs2;
LWextFS myext4fs3;
LWextFS myext4fs4;

#define sda1 0	// First partition on first USB device.
#define sda2 1	// Second partition on first USB device.
#define sdb1 4	// First partition on second USB device.
#define sdb2 5	// Second partition on second USB device.
#define sdc1 8	// First partition on third USB device.
#define sdd1 12  // First partition on SD device

// Create ext4FS source and destination file pointers.
File file1; // src
File file2; // dest

// Grab a pointer to the mount list.
bd_mounts_t *ml = get_mount_list();

// File to copy. This file is provided in the extras folder in ths library.
// Change this to any other file you wish to copy.
const char *file2Copy = "32MEGfile.dat";

#define BUFSIZE (32*1024)  // Buffer size. Play with this:)
uint32_t buffer[BUFSIZE];  // File copy buffer

// Copy a file from one drive to another.
// Set 'stats' to true to display a progress bar,
// copy speed and copy time. 
int fileCopy(bool stats) {

    int br = 0, bw = 0;          // File read/write count
	uint32_t cntr = 0;
	uint32_t start = 0, finish = 0;
	uint32_t bytesRW = 0;
	int copyError = 0;
	
    /* Copy source to destination */
	start = micros();
    for (;;) {
		if(stats) { // If true, display progress bar.
			cntr++;
			if(!(cntr % 10)) Serial.printf("*");
			if(!(cntr % 640)) Serial.printf("\n");
		}
		br = file1.read(buffer, sizeof(buffer));  // Read buffer size of source file (USB Type)
        if (br <= 0) {
			copyError = br;
			break; // Error or EOF
		}
		bw = file2.write(buffer, br); // Write it to the destination file (USB Type)
        if (bw < br) {
			copyError = bw; // Error or disk is full
			break;
		}
		bytesRW += (uint32_t)bw;
    }
	file2.flush(); // Flush write buffer.
    // Close open files
	file1.close(); // Source
	file2.close(); // Destination
	finish = (micros() - start); // Get total copy time.
    float MegaBytes = (bytesRW*1.0f)/(1.0f*finish);
	if(stats) // If true, display time stats.
		Serial.printf("\nCopied %u bytes in %f seconds. Speed: %f MB/s\n",
		                 bytesRW,(finish*1.0f)/1000000.0,MegaBytes);
	return copyError; // Return any errors or success.
}

void listDirectories(void) {
	if(ml[sda1].mounted) {
		Serial.printf("-------------------------------------------------\n");
		Serial.printf("sda1 directory listing:\n");
		Serial.printf("Volume Name: %s\n",myext4fs1.getMediaName());
		printDirectory(myext4fs1);
		Serial.printf("Bytes Used: %llu, Bytes Total:%llu\n\n", myext4fs1.usedSize(), myext4fs1.totalSize());
	}
	if(ml[sdb1].mounted) {
		Serial.printf("sdb1 directory listing:\n");
		Serial.printf("Volume Name: %s\n",myext4fs2.getMediaName());
		printDirectory(myext4fs2);
		Serial.printf("Bytes Used: %llu, Bytes Total:%llu\n\n", myext4fs2.usedSize(), myext4fs2.totalSize());
	}
	if(ml[sdd1].mounted) {
		Serial.printf("sdd1 card directory listing:\n");
		Serial.printf("Volume Name: %s\n",myext4fs3.getMediaName());
		printDirectory(myext4fs3);
		Serial.printf("Bytes Used: %llu, Bytes Total:%llu\n\n", myext4fs3.usedSize(), myext4fs3.totalSize());
	}
	Serial.printf("-------------------------------------------------\n");
}

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
     yield(); // wait for serial port to connect.
  }
  if(CrashReport)
	Serial.print(CrashReport);

//ext4_dmask_set(DEBUG_ALL);

  // Start USBHost_t36, HUB(s) and USB devices.
  myusb.begin();

  Serial.printf("%cLWEXT4 MULTI USB DRIVE AND SD CARD FILE COPY TESTING\n\n",12);
  Serial.println("Initializing LWextFS ...");

  Serial.println("Initializing all availble lwext devices.\n");
  Serial.println("Please Wait...\n");
  int devcount = myext4fs.lwext_init_devices();  
  Serial.print(devcount,DEC);
  Serial.println(" lwext devices found.\n\n");

  if(!myext4fs1.begin(sda1)) {
    Serial.println("myext4fs.begin(sda1) Failed: Drive plugged in?");
  } else {
    Serial.println("myext4fs.begin(sda1): passed...");
  }
 
  if(!myext4fs2.begin(sdb1)) {
    Serial.println("myext4fs.begin(sdb1) Failed: Drive plugged in?");
  } else {
    Serial.println("myext4fs.begin(sdb1): passed...");
  }

  if(!myext4fs3.begin(sdd1)) {
    Serial.println("myext4fs.begin(sdd1) Failed: Drive plugged in?");
  } else {
    Serial.println("myext4fs.begin(sdd1): passed...");
  }
}

void loop(void) {
	uint8_t c = 0;
	int copyResult = 0;

	Serial.printf("\n------------------------------------------------------------------\n");
	Serial.printf("Select:\n");
	Serial.printf("   1)  to copy '%s' from USB drive 1 to USB drive 2.\n", file2Copy);
	Serial.printf("   2)  to copy '%s' from USB drive 2 to USB drive 1.\n", file2Copy);
	Serial.printf("   3)  to copy '%s' from USB drive 1 to SDIO card.\n", file2Copy);
	Serial.printf("   4)  to copy '%s' from USB drive 2 to SDIO card.\n", file2Copy);
	Serial.printf("   5)  to copy '%s' from SDIO card to USB drive 1.\n", file2Copy);
	Serial.printf("   6)  to copy '%s' from SDIO card to USB drive 2.\n", file2Copy);
	Serial.printf("   7)  List Directories\n");
	Serial.printf("   8)  UnMount drives and quit.\n");
	Serial.printf("------------------------------------------------------------------\n");

	while(!Serial.available());
	c = Serial.read();
	while(Serial.available()) Serial.read(); // Get rid of CR and/or LF if there.

	// This is a rather large and bloated switch() statement. And there are better ways to do this
	// but it served the quick copy paste modify senario:)
	switch(c) {
		case '1':
			if(!ml[sda1].mounted || !ml[sdb1].mounted) {
				Serial.println("\nsda1 or sdb1 is not mounted...");
				break;
			}
			Serial.printf("\n1) Copying from USB drive 1 to USB drive 2\n");
			// Attempt to open source file
			if(!(file1 = myext4fs1.open(file2Copy, FILE_READ))) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			// Attempt to create destination file
			if(!(file2 = myext4fs2.open(file2Copy, FILE_WRITE_BEGIN))) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;

		case '2':
			if(!ml[sdb1].mounted || !ml[sda1].mounted) {
				Serial.println("\nsda1 or sdb1 is not mounted...");
				break;
			}
			Serial.printf("\n2) Copying from USB drive 2 to USB drive 1\n");
			if(!(file1 = myext4fs2.open(file2Copy, FILE_READ))) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			if(!(file2 = myext4fs1.open(file2Copy, FILE_WRITE_BEGIN))) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;

		case '3':
			if(!ml[sda1].mounted || !ml[sdd1].mounted) {
				Serial.println("\nsda1 or sdd1 is not mounted...");
				break;
			}
			Serial.printf("\n3) Copying from USB drive 1 to SDIO card\n");
			if(!(file1 = myext4fs1.open(file2Copy, FILE_READ))) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			if(!(file2 = myext4fs3.open(file2Copy, FILE_WRITE_BEGIN))) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;

		case '4':
			if(!ml[sdb1].mounted || !ml[sdd1].mounted) {
				Serial.println("\nsdb1 or sdd1 is not mounted...");
				break;
			}
			Serial.printf("\n4) Copying from USB drive 2 to SDIO card\n");
			if(!(file1 = myext4fs2.open(file2Copy, FILE_READ))) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			if(!(file2 = myext4fs3.open(file2Copy, FILE_WRITE_BEGIN))) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;
		case '5':
			if(!ml[sdd1].mounted || !ml[sda1].mounted) {
				Serial.println("\nsdd1 or sda1 is not mounted...");
				break;
			}
			Serial.printf("\n5) Copying from SDIO card to USB drive 1 \n");
			if(!(file1 = myext4fs3.open(file2Copy, FILE_READ))) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			if(!(file2 = myext4fs1.open(file2Copy, FILE_WRITE_BEGIN))) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;
		case '6':
			if(!ml[sdd1].mounted || !ml[sdb1].mounted) {
				Serial.println("\nsdd1 or sdb1 is not mounted...");
				break;
			}
			Serial.printf("\n6) Copying from SDIO card to USB drive 2\n");
			if(!(file1 = myext4fs3.open(file2Copy, FILE_READ))) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			if(!(file2 = myext4fs2.open(file2Copy, FILE_WRITE_BEGIN))) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;
		case '7':
			listDirectories();
			break;
		case '8':
			if(ml[sda1].mounted) {
				Serial.println("unmounting sda1...");
				myext4fs1.lwext_umount(sda1);
			}
			if(ml[sdb1].mounted) {
				Serial.println("unmounting sda1...");
				myext4fs2.lwext_umount(sdb1);
			}
			if(ml[sdd1].mounted) {
				Serial.println("unmounting sdd1...");
				myext4fs3.lwext_umount(sdd1);
			}
			Serial.println("Done...");
			while(1);
		default:
			break;
	}
}

void printDirectory(FS &fs) {
  printDirectory(fs.open("/"), 0);
  Serial.println();
}

void printDirectory(File dir, int numSpaces) {
   while(true) {
     File entry = dir.openNextFile();
     if (! entry) {
       //Serial.println("** no more files **");
       break;
     }
     printSpaces(numSpaces);
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numSpaces+2);
     } else {
       // files have sizes, directories do not
       printSpaces(40 - numSpaces - strlen(entry.name()));
       Serial.print("  ");
       Serial.print(entry.size(), DEC);
       DateTimeFields datetime;
       if (entry.getModifyTime(datetime)) {
         printSpaces(10);
         printTime(datetime);
       }
       Serial.println();
     }
     entry.close();
   }
}

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}

void printTime(const DateTimeFields tm) {
  const char *months[12] = {
    "January","February","March","April","May","June",
    "July","August","September","October","November","December"
  };
  if (tm.hour < 10) Serial.print('0');
  Serial.print(tm.hour);
  Serial.print(':');
  if (tm.min < 10) Serial.print('0');
  Serial.print(tm.min);
  Serial.print("  ");
  Serial.print(tm.mon < 12 ? months[tm.mon] : "???");
  Serial.print(" ");
  Serial.print(tm.mday);
  Serial.print(", ");
  Serial.print(tm.year + 1900);
}
