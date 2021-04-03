// mscHost.cpp

#include "Arduino.h"
#include "ext4_errno.h"
#include "ext4MscHost.h"
#include "USBHost_t36.h"

// Setup USBHost_t36 and as many HUB ports as needed.
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);

msController msDrive1(myusb);
//msController msDrive2(myusb);

// Setup debugging LED pin defs.
// Used mainly to see  the activity of non-blocking reads and writes.
#define WRITE_PIN			33		// Pin number of drive read activity led (RED LED).
#define READ_PIN			34		// Pin number of drive read activity led (GREEN LED).

void switchGreenLed(bool onOff) {
	if(onOff)
		digitalWriteFast(READ_PIN, HIGH); // If so, turn on the red LED and proceed with dispatch.
	else
		digitalWriteFast(READ_PIN, LOW); // If so, turn on the red LED and proceed with dispatch.
}
void switchRedLed(bool onOff) {
	if(onOff)
		digitalWriteFast(WRITE_PIN, HIGH); // If so, turn on the red LED and proceed with dispatch.
	else
		digitalWriteFast(WRITE_PIN, LOW); // If so, turn on the red LED and proceed with dispatch.
}

void printValue(uint32_t value) {
	Serial.printf("value = %d\n",value);
}

int mscInit(void) {
//Serial.printf("mscInit()\n");
	pinMode(READ_PIN, OUTPUT); // Init disk read activity indicator.
	pinMode(WRITE_PIN, OUTPUT); // Init disk write activity indicator.
	
	if(msDrive1.mscInit() != MS_CBW_PASS)
		return EIO;
	else
	    return EOK;
}

uint32_t getBlockSize(void) {
	return msDrive1.msCapacity.BlockSize;
}

uint32_t getBlockCount(void) {
	return msDrive1.msCapacity.Blocks;
}

void mscHostInit(void) {
	// Start USBHost_t36, HUB(s) and USB devices.
	myusb.begin();
}

int checkConnectedInitialized(void) {
	if(msDrive1.checkConnectedInitialized() != MS_CBW_PASS)
		return EIO;
	else
	    return EOK;
   
}

bool mscAvailable(void) {
	return msDrive1.mscTransferComplete;
}

uint8_t msReadBlocks(const uint32_t BlockAddress, const uint16_t Blocks, const uint16_t BlockSize, void * sectorBuffer) {
	uint8_t msResult = msDrive1.msReadBlocks(BlockAddress, Blocks, BlockSize, sectorBuffer);
	return msResult;
}

uint8_t msWriteBlocks(const uint32_t BlockAddress, const uint16_t Blocks, const uint16_t BlockSize, const void * sectorBuffer) {
	uint8_t msResult = msDrive1.msWriteBlocks(BlockAddress, Blocks, BlockSize, sectorBuffer);
	return msResult;
}
