/**************************************************************
 * This program is a simple binary write/read benchmark.      *
 * Loosely based on Bill Greimans Bench.ino sketch for SdFat. *
 * Modified to work with MSC and TeensyEXT4.                  *
 * ************************************************************
 */
#include <USBHost_t36.h>
#include "ext4FS.h"
#include "sdios.h"
#include "FreeStack.h"

// Setup USBHost_t36 and as many HUB ports as needed.
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);

// Setup MSC for the number of USB Drives you are using. (Two for this example)
// Mutiple  USB drives can be used. Hot plugging is supported. There is a slight
// delay after a USB MSC device is plugged in. This is waiting for initialization
// but after it is initialized ther should be no delay.
USBDrive ext4Drive1(myusb);

#define sda1 0	// First partition on first USB EXT4 device.

// Set SKIP_FIRST_LATENCY true if the first read/write to the SD can
// be avoid by writing a file header or reading the first record.
const bool SKIP_FIRST_LATENCY = false;

// Size of read/write buffer.
const size_t BUF_SIZE = 32*1024;

// File size in MB where MB = 1,000,000 bytes.
const uint32_t FILE_SIZE_MB = 32;

// Write pass count.
const uint8_t WRITE_COUNT = 2;

// Read pass count.
const uint8_t READ_COUNT = 2;
//==============================================================================
// End of configuration constants.
//------------------------------------------------------------------------------
// File size in bytes.
//const uint32_t FILE_SIZE = 1000000UL*FILE_SIZE_MB;
const uint32_t FILE_SIZE = 1024000UL*FILE_SIZE_MB;

// Insure 4-byte alignment.
uint32_t buf32[(BUF_SIZE + 3)/4];
uint8_t* buf = (uint8_t*)buf32;

// create one instance of ext4 file system.
ext4FS myext4fs1;
// Grab a pointer to the mount list.
bd_mounts_t *ml = myext4fs1.get_mount_list();
// Setup a FS file pointer.
File file;

// Serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) Serial.println(s)
void setup() {
  // Wait for USB Serial
  while (!Serial) {
    yield();
  }

  // Start  USBHost_t36
  myusb.begin();
  delay(500); // Give drives time to come online (you may have to adjust this for certain drives).
  myusb.Task(); // Start looking for available drives.

  // Initialize EXT4 physical block device.
  myext4fs1.init_block_device(&ext4Drive1, sda1);  

  cout << F("\nUse a freshly formatted Mass Storage drive for best performance.\n");
  Serial.printf("\n%cInitializing USB MSC drive...",12);

  // Start ext4FS filesystem.
  if(!myext4fs1.begin(sda1)) {
    Serial.printf("myext4fs.begin(%s) Failed: Drive plugged in?\n",mpName[sda1]);
    Serial.printf("Make sure USB drive is formatted as EXT4 and plugged in. Then restart sketch...\n");
    while(1);  // Give up
  } else {
    Serial.printf("myext4fs.begin(%s): passed...\n", mpName[sda1]);
  }

  Serial.printf("Volume Name: %s\n",myext4fs1.getVolumeLabel());
  // use uppercase in hex and use 0X base prefix
  cout << uppercase << showbase << endl;
}
//------------------------------------------------------------------------------
void loop() {
  float s;
  uint32_t t;
  uint32_t maxLatency;
  uint32_t minLatency;
  uint32_t totalLatency;
  bool skipLatency;

  // Discard any residual input.
  do {
    delay(10);
  } while (Serial.available() && Serial.read() >= 0);

  // F() stores strings in flash to save RAM
  cout << F("Type any character to (re)start or 'e'/'E' to unmount and end...\n");
  while (!Serial.available()) {
    yield();
  }
  char c = Serial.read();
  if(c == 'e' || c == 'E') {
    myext4fs1.lwext_umount(sda1);
    Serial.println("\next4 drive unmounted");
    Serial.println("EXT4 benchUSB finished...");
    while(1);
  }
#if HAS_UNUSED_STACK
  cout << F("FreeStack: ") << FreeStack() << endl;
#endif  // HAS_UNUSED_STACK

  if (ml[sda1].pt == EXT4_TYPE) {
    cout << F("Type is EXT4") << endl;
  } else {
    cout << F("Error type is FAT or EXFAT") << endl;
    while(1);
  }

  cout << F("Card size: ") << myext4fs1.totalSize();
  cout << F(" GB (GB = 1E9 bytes)") << endl;

  // open or create file - truncate existing file.
  file = myext4fs1.open("bench.dat", FILE_WRITE_BEGIN);
  if (!file) {
    error("open failed");
  }

  // fill buf with known data
  if (BUF_SIZE > 1) {
    for (size_t i = 0; i < (BUF_SIZE - 2); i++) {
      buf[i] = 'A' + (i % 26);
    }
    buf[BUF_SIZE-2] = '\r';
  }
  buf[BUF_SIZE-1] = '\n';

  cout << F("FILE_SIZE_MB = ") << FILE_SIZE_MB << endl;
  cout << F("BUF_SIZE = ") << BUF_SIZE << F(" bytes\n");
  cout << F("Starting write test, please wait.") << endl << endl;

  // do write test
  uint32_t n = FILE_SIZE/BUF_SIZE;
  cout <<F("write speed and latency") << endl;
  cout << F("speed,max,min,avg") << endl;
  cout << F("KB/Sec,usec,usec,usec") << endl;
  for (uint8_t nTest = 0; nTest < WRITE_COUNT; nTest++) {
    file.truncate(0);
    maxLatency = 0;
    minLatency = 9999999;
    totalLatency = 0;
    skipLatency = SKIP_FIRST_LATENCY;
    t = millis();
    for (uint32_t i = 0; i < n; i++) {
      uint32_t m = micros();
      if (file.write(buf, BUF_SIZE) != BUF_SIZE) {
        error("write failed");
      }
      m = micros() - m;
      totalLatency += m;
      if (skipLatency) {
        // Wait until first write to MSC drive, not just a copy to the cache.
        skipLatency = file.position() < 512;
      } else {
        if (maxLatency < m) {
          maxLatency = m;
        }
        if (minLatency > m) {
          minLatency = m;
        }
      }
    }
    file.flush();
    t = millis() - t;
    s = file.size();
    cout << s/t <<',' << maxLatency << ',' << minLatency;
    cout << ',' << totalLatency/n << endl;
  }
  cout << endl << F("Starting read test, please wait.") << endl;
  cout << endl <<F("read speed and latency") << endl;
  cout << F("speed,max,min,avg") << endl;
  cout << F("KB/Sec,usec,usec,usec") << endl;

  // do read test
  for (uint8_t nTest = 0; nTest < READ_COUNT; nTest++) {
    file.seek(0);
    maxLatency = 0;
    minLatency = 9999999;
    totalLatency = 0;
    skipLatency = SKIP_FIRST_LATENCY;
    t = millis();
    for (uint32_t i = 0; i < n; i++) {
      buf[BUF_SIZE-1] = 0;
      uint32_t m = micros();
      int32_t nr = file.read(buf, BUF_SIZE);
      if (nr != BUF_SIZE) {
        error("read failed");
      }
      m = micros() - m;
      totalLatency += m;
      if (buf[BUF_SIZE-1] != '\n') {

        error("data check error");
      }
      if (skipLatency) {
        skipLatency = false;
      } else {
        if (maxLatency < m) {
          maxLatency = m;
        }
        if (minLatency > m) {
          minLatency = m;
        }
      }
    }
    s = file.size();
    t = millis() - t;
    cout << s/t <<',' << maxLatency << ',' << minLatency;
    cout << ',' << totalLatency/n << endl;
  }
  cout << F("Filesize = ") << s << endl;
  cout << endl << F("Done testing") << endl;
  file.close();
}
