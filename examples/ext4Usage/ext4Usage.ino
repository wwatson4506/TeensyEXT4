// ext4Usage.ino  Teensy lwext4 testing 
// Based on lwext4 by: Grzegorz Kostka (kostka.grzegorz@gmail.com)
// Modified version of LittlFS_Usage.ino for use with LWextFS.

#include "ext4FS.h"

extern USBHost myusb; // Located and defined in 'ext4FS.cpp'.

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
// Setup four instances of LWextFS (four mountable partittions).
//**********************************************************************
LWextFS myext4fs;
LWextFS myext4fs1;
LWextFS myext4fs2;
LWextFS myext4fs3;
LWextFS myext4fs4;

File file, file1, file2;

#define sda1 0	// First partition on first USB device.
#define sda2 1	// Second partition on first USB device.
#define sdb1 4	// First partition on second USB device.
#define sdb2 5	// Second partition on second USB device.
#define sdc1 8	// First partition on third USB device.
#define sdd1 12  // First partition on SD device

// Set this to one of the above devices.
#define sdxx sda1

void setup() {
   while (!Serial) {
    yield(); // wait for serial port to connect.
  }
  if(CrashReport)
	Serial.print(CrashReport);
//ext4_dmask_set(DEBUG_ALL);

  Serial.printf("%cTeensy lwext file system test\n\n",12);
  Serial.println("Initializing LWextFS ...");

  myusb.begin();

  Serial.println("Initializing device sdxx.\n");
  myext4fs.init_block_device(sdxx);  

  if(!myext4fs1.begin(sdxx)) { // Change this to sdd1 for SD card.
    Serial.printf("myext4fs.begin(sdxx) Failed: Drive plugged in?\n");
	while(1); // Give up !!!
  } else {
    Serial.printf("myext4fs.begin(sdxx): passed...\n");
  }

  Serial.printf("Volume Name: %s\n",myext4fs1.getMediaName());

  // Now lets create a file and write some data.  Note: basically the same usage for 
  // creating and writing to a file using SD library.
  Serial.println("\n---------------");
  Serial.println("Now lets create a file with some data in it");
  Serial.println("---------------");
  char someData[8192];
  memset( someData, 'z', 8192 );
  file = myext4fs1.open("bigfile.txt", FILE_WRITE_BEGIN);
  file.write(someData, sizeof(someData));

  for (uint16_t j = 0; j < 1000; j++)
    file.write(someData, sizeof(someData));
  file.close();
  // We can also get the size of the file just created.  Note we have to open and 
  // thes close the file unless we do file size before we close it in the previous step
  file = myext4fs1.open("bigfile.txt", FILE_WRITE);
  Serial.printf("File Size of bigfile.txt (bytes): %u\n", file.size());
  file.close();

  // Now that we initialized the FS and created a file lets print the directory.
  // Note:  Since we are going to be doing print directory and getting disk usuage
  // lets make it a function which can be copied and used in your own sketches.
  listFiles();
  waitforInput();

  // Now lets rename the file
  Serial.println("\n---------------");
  Serial.println("Rename bigfile to file10");
  myext4fs1.rename("bigfile.txt", "file10.txt");
  listFiles();
  waitforInput();

  // To delete the file
  Serial.println("\n---------------");
  Serial.println("Delete file10.txt");
  myext4fs1.remove("file10.txt");
  listFiles();
  waitforInput();

  Serial.println("\n---------------");
  Serial.println("Create a directory and a subfile");
  myext4fs1.mkdir("structureData1");

  file = myext4fs1.open("structureData1/temp_test.txt", FILE_WRITE);
  file.println("SOME DATA TO TEST");
  file.close();
  listFiles();
  waitforInput();

  Serial.println("\n---------------");
  Serial.println("Rename directory");
  myext4fs1.rename("structureData1", "structuredData");
  listFiles();
  waitforInput();

  Serial.println("\n---------------");
  Serial.println("Lets remove them now...");
  //Note have to remove directories files first
  myext4fs1.remove("structuredData1/temp_test.txt");
  myext4fs1.rmdir("structuredData");
  listFiles();
  waitforInput();

  Serial.println("\n---------------");
  Serial.println("Now lets create a file and read the data back...");
  
  // LWextFS also supports truncate function similar to SDFat. As shown in this
  // example, you can truncate files.
  //
  Serial.println();
  Serial.println("Writing to datalog.bin using LWextFS functions");
  file1 = myext4fs1.open("datalog.bin", FILE_WRITE);
  unsigned int len = file1.size();
  Serial.print("datalog.bin started with ");
  Serial.print(len);
  Serial.println(" bytes");
  if (len > 0) {
    // reduce the file to zero if it already had data
    file1.truncate();
  }
  file1.print("Just some test data written to the file (by LWextFS functions)");
  file1.write((uint8_t) 0);
  file1.close();

  // You can also use regular SD type functions, even to access the same file.  Just
  // remember to close the file before opening as a regular SD File.
  //
  Serial.println();
  Serial.println("Reading to datalog.bin using LWextFS functions");
  file2 = myext4fs1.open("datalog.bin");
  if (file2) {
    char mybuffer[100];
    int index = 0;
    while (file2.available()) {
      char c = file2.read();
      mybuffer[index] = c;
      if (c == 0) break;  // end of string
      index = index + 1;
      if (index == 99) break; // buffer full
    }
    mybuffer[index] = 0;
    Serial.print("  Read from file: ");
    Serial.println(mybuffer);
  } else {
    Serial.println("unable to open datalog.bin :(");
  }
  file2.close();
  Serial.println("unmounting sdxx...");
  myext4fs1.lwext_umount(sdxx);
  
  Serial.println("\nBasic Usage Example Finished");
}

void loop() {

}

void listFiles()
{
  Serial.println("---------------");
  printDirectory(myext4fs1);
  Serial.printf("Bytes Used: %llu, Bytes Total:%llu\n", myext4fs1.usedSize(), myext4fs1.totalSize());
}

void printDirectory(FS &fs) {
  Serial.println("Directory\n---------");
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
       printSpaces(36 - numSpaces - strlen(entry.name()));
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

void waitforInput()
{
  Serial.println("Press anykey to continue");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
