// wavePlayerExt4.ino  Teensy lwext4 testing 
// Based on lwext4 by: Grzegorz Kostka (kostka.grzegorz@gmail.com)
// Modified version of WavFilePlayerUSB.ino for use with TeensyEXT4.

#include <USBHost_t36.h>
#include <Audio.h>
#include "play_FS_wav.h" // Should be included in 'Audio.h'
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
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

File file, file1, file2;

#define sda1 0	// First partition on first USB device.
#define sda2 1	// Second partition on first USB device.
#define sdb1 4	// First partition on second USB device.
#define sdc1 8	// First partition on third USB device.
#define sdd1 12  // First partition on SD device

// Set this to one of the above devices.
#define sdxx sda1

AudioPlayFSWav           playWav1;
// Use one of these 3 output types: Digital I2S, Digital S/PDIF, or Analog DAC
AudioOutputI2S           audioOutput;
//AudioOutputSPDIF       audioOutput;
//AudioOutputAnalog      audioOutput;
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;

void setup() {
   while (!Serial) {
    yield(); // wait for serial port to connect.
  }
  if(CrashReport)
	Serial.print(CrashReport);

  Serial.printf("%cTeensy lwext WavFilePlayer\n\n",12);
  Serial.println("Initializing ext4FS ...");
  Serial.println("Please wait...");


  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(8);

  // Comment these out if not using the audio adaptor board.
  // This may wait forever if the SDA & SCL pins lack
  // pullup resistors
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.50);

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

  if(!myext4fs1.begin(sdxx)) { // Change this to sdd1 for SD card.
    Serial.printf("myext4fs.begin(%s) Failed: Drive plugged in?\n",mpName[sdxx]);
	while(1); // Give up !!!
  } else {
    Serial.printf("myext4fs.begin(%s): passed...\n",mpName[sdxx]);
  }
}

void playFile(const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav1.play(filename);

  // A brief delay for the library read WAV info
  delay(5);

  // Simply wait for the file to finish playing.
  while (playWav1.isPlaying()) {
    // uncomment these lines if you audio shield
    // has the optional volume pot soldered
    float vol = analogRead(15);
    vol = vol / 1024;
    sgtl5000_1.volume(vol);
  }
}

void loop() {
  listFiles();
  waitforInput();
  playFile("YoureLazyJimmyBarnesJoeBonamassa.wav");  // filenames are always uppercase 8.3 format
  delay(500);
  playFile("SDTEST1.WAV");  // filenames are always uppercase 8.3 format
  delay(500);
  playFile("SDTEST2.WAV");
  delay(500);
  playFile("SDTEST3.WAV");
  delay(500);
  playFile("SDTEST4.WAV");
  delay(1500);
}

void listFiles()
{
  Serial.println("---------------");
  printDirectory(myext4fs1);
  Serial.printf("Bytes Used: %llu, Bytes Total:%llu\n", myext4fs1.usedSize(), myext4fs1.totalSize());
}

void printDirectory(FS &fs) {
  Serial.println("Directory\n---------");
  printDirectory(fs.open(""), 0);
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
