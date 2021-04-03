// mscHost.h

#ifndef BLOCKDEV_H_
#define BLOCKDEV_H_


#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif
//void mscHostInit(void);
int mscInit(void);
uint8_t msGetMaxLun(void);
//void msCurrentLun(uint8_t lun) {currentLUN = lun;}
//uint8_t msCurrentLun() {return currentLUN;}
//bool available() { delay(0); return deviceAvailable; }
int checkConnectedInitialized(void);
//uint16_t getIDVendor() {return idVendor; }
//uint16_t getIDProduct() {return idProduct; }
//uint8_t getHubNumber() { return hubNumber; }
//uint8_t getHubPort() { return hubPort; }
//uint8_t getDeviceAddress() { return deviceAddress; }
uint8_t WaitMediaReady();
uint8_t msTestReady();
uint8_t msReportLUNs(uint8_t *Buffer);
uint8_t msStartStopUnit(uint8_t mode);
//uint8_t msReadDeviceCapacity(msSCSICapacity_t * const Capacity);
//uint8_t msDeviceInquiry(msInquiryResponse_t * const Inquiry);
//uint8_t msRequestSense(msRequestSenseResponse_t * const Sense);
uint8_t msRequestSense(void *Sense);
uint8_t msReadBlocks(const uint32_t BlockAddress, const uint16_t Blocks,
					 const uint16_t BlockSize, void * sectorBuffer);
uint8_t msReadSectorsWithCB(const uint32_t BlockAddress, const uint16_t Blocks, void (*callback)(uint32_t token, uint8_t* data), uint32_t token);
uint8_t msWriteBlocks(const uint32_t BlockAddress, const uint16_t Blocks,
                      const uint16_t BlockSize,	const void * sectorBuffer);
uint32_t getBlockSize(void);
uint32_t getBlockCount(void);
void switchGreenLed(bool onOff);
void switchRedLed(bool onOff);
void printValue(uint32_t value);
bool mscAvailable(void);

#ifdef __cplusplus
}
#endif

//void mscHostInit(void);
int mscInit(void);
uint8_t msGetMaxLun(void);
//void msCurrentLun(uint8_t lun) {currentLUN = lun;}
//uint8_t msCurrentLun() {return currentLUN;}
//bool available() { delay(0); return deviceAvailable; }
int checkConnectedInitialized(void);
//uint16_t getIDVendor() {return idVendor; }
//uint16_t getIDProduct() {return idProduct; }
//uint8_t getHubNumber() { return hubNumber; }
//uint8_t getHubPort() { return hubPort; }
//uint8_t getDeviceAddress() { return deviceAddress; }
uint8_t WaitMediaReady();
uint8_t msTestReady();
uint8_t msReportLUNs(uint8_t *Buffer);
uint8_t msStartStopUnit(uint8_t mode);
//uint8_t msReadDeviceCapacity(msSCSICapacity_t * const Capacity);
//uint8_t msDeviceInquiry(msInquiryResponse_t * const Inquiry);
//uint8_t msRequestSense(msRequestSenseResponse_t * const Sense);
uint8_t msRequestSense(void *Sense);
uint8_t msReadBlocks(const uint32_t BlockAddress, const uint16_t Blocks,
					 const uint16_t BlockSize, void * sectorBuffer);
uint8_t msReadSectorsWithCB(const uint32_t BlockAddress, const uint16_t Blocks, void (*callback)(uint32_t token, uint8_t* data), uint32_t token);
uint8_t msWriteBlocks(const uint32_t BlockAddress, const uint16_t Blocks,
                      const uint16_t BlockSize,	const void * sectorBuffer);
uint32_t getBlockSize(void);
uint32_t getBlockCount(void);
void switchGreenLed(bool onOff);
void switchRedLed(bool onOff);
void printValue(uint32_t value);
bool mscAvailable(void);

#endif
