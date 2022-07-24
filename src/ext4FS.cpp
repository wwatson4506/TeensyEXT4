/* ext4fs library compatibility wrapper for use of lwext on Teensy
 * Copyright (c) 2022, Warren Watson, wwatson4506@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Arduino.h>
#include <ext4FS.h>

ext4FS EXT;
// Setup USBHost_t36 and as many HUB ports as needed.
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);

// Instances for the number of USB drives you are using.
USBDrive myDrive1(myusb);
USBDrive myDrive2(myusb);
USBDrive myDrive3(myusb);
USBDrive *device_list[] = {&myDrive1, &myDrive2, &myDrive3};

// SdFat usage with an SD card.
#define SD_CONFIG SdioConfig(FIFO_SDIO)
SdCardFactory cardFactory;
SdCard *sd;

/**@brief   Block size.*/
#define BLOCK_SIZE 512

//******************************************************************************
// An array of parent block devices.
//******************************************************************************
static block_device_t bd_list[CONFIG_EXT4_BLOCKDEVS_COUNT];
//******************************************************************************
// An array of mounted partitions.
//******************************************************************************
static bd_mounts_t mount_list[MAX_MOUNT_POINTS];

//**********************BLOCKDEV INTERFACE**************************************
static int ext4_bd_open(struct ext4_blockdev *bdev);
static int ext4_bd_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
			 uint32_t blk_cnt);
static int ext4_bd_bwrite(struct ext4_blockdev *bdev, const void *buf,
			  uint64_t blk_id, uint32_t blk_cnt);
static int ext4_bd_close(struct ext4_blockdev *bdev);
static int ext4_bd_lock(struct ext4_blockdev *bdev);
static int ext4_bd_unlock(struct ext4_blockdev *bdev);

//******************************************************************************
// The list of low level parent block device instances.
//******************************************************************************
EXT4_BLOCKDEV_STATIC_INSTANCE(_ext4_bd,  BLOCK_SIZE, 0, ext4_bd_open,
			      ext4_bd_bread, ext4_bd_bwrite, ext4_bd_close,
			      0, 0);
#if CONFIG_EXT4_BLOCKDEVS_COUNT > 1
EXT4_BLOCKDEV_STATIC_INSTANCE(_ext4_bd1,  BLOCK_SIZE, 0, ext4_bd_open,
			      ext4_bd_bread, ext4_bd_bwrite, ext4_bd_close,
			      0, 0);
#endif
#if CONFIG_EXT4_BLOCKDEVS_COUNT > 2
EXT4_BLOCKDEV_STATIC_INSTANCE(_ext4_bd2,  BLOCK_SIZE, 0, ext4_bd_open,
			      ext4_bd_bread, ext4_bd_bwrite, ext4_bd_close,
			      0, 0);
#endif
#if CONFIG_EXT4_BLOCKDEVS_COUNT > 3
EXT4_BLOCKDEV_STATIC_INSTANCE(_ext4_bd3,  BLOCK_SIZE, 0, ext4_bd_open,
			      ext4_bd_bread, ext4_bd_bwrite, ext4_bd_close,
			      0, 0);
#endif

//******************************************************************************
// List of block device interfaces.
//******************************************************************************
static struct ext4_blockdev * const ext4_blkdev_list[CONFIG_EXT4_BLOCKDEVS_COUNT] =
{
	&_ext4_bd,
#if CONFIG_EXT4_BLOCKDEVS_COUNT > 1
	&_ext4_bd1,
#endif
#if CONFIG_EXT4_BLOCKDEVS_COUNT > 2
	&_ext4_bd2,
#endif
#if CONFIG_EXT4_BLOCKDEVS_COUNT > 3
	&_ext4_bd3,
#endif
};

// Dump mount list. mounted/unmounted
void dumpMountList(void) {
	for (int i = 0; i < 16; i++) {
		if(mount_list[i].parent_bd.connected && (strcmp(mount_list[i].pname,"UnKnown") != 0)) {
			Serial.printf("mount_list[%d].pname = %s\n", i, mount_list[i].pname);
			Serial.printf("mount_list[%d].available = %d\n", i, mount_list[i].available);
			Serial.printf("mount_list[%d].parent_bd.name = %s\n", i, mount_list[i].parent_bd.name);
			if(mount_list[i].mounted)
				Serial.printf("mount_list[%d].mounted = Mounted\n\n", i);
			else
				Serial.printf("mount_list[%d].mounted = UnMounted\n\n", i);
		}
	}
}

// Dump block device list.
void dumpBDList(void) {
	for (int i = 0; i < CONFIG_EXT4_BLOCKDEVS_COUNT; i++) {
		if(mount_list[i].parent_bd.connected) {
			Serial.printf("bd_list[%d].dev_id = %d\n", i, bd_list[i].dev_id);
			Serial.printf("bd_list[%d].name = %s\n", i, bd_list[i].name);
			Serial.printf("bd_list[%d].*pDrive = %d (USB)\n", i, bd_list[i].pDrive);
			Serial.printf("bd_list[%d].*pSD = %d (SD)\n", i, bd_list[i].pSD);
			Serial.printf("bd_list[%d].*pbdev = %d\n", i, bd_list[i].pbdev);
			if(bd_list[i].connected)
			Serial.printf("bd_list[%d].connected = true\n\n",i);
			else
			Serial.printf("bd_list[%d].connected = false\n\n",i);
		}
	}
}

//******************************************************************************
// Open block device. Low level and partition.
//******************************************************************************
static int ext4_bd_open(struct ext4_blockdev *bdev)
{
	int index;
	myusb.Task();
	index = get_bdev(bdev);
	if(index == -1)
		index = get_device_index(bdev);
	if(!bd_list[index].pDrive->filesystemsStarted())
		bd_list[index].pDrive->startFilesystems(); 
	if(index <= 2) {
		if(bd_list[index].pDrive->checkConnectedInitialized()) return EIO;
		bd_list[index].pbdev->part_offset = 0;
		bd_list[index].pbdev->part_size = bd_list[index].pDrive->msDriveInfo.capacity.Blocks *
							bd_list[index].pDrive->msDriveInfo.capacity.BlockSize;
		bd_list[index].pbdev->bdif->ph_bcnt = bd_list[index].pDrive->msDriveInfo.capacity.Blocks;
	} else {
		if(!bd_list[index].pSD) return EIO;
		bd_list[index].pbdev->part_offset = 0;
		bd_list[index].pbdev->part_size = bd_list[index].pSD->sectorCount() * BLOCK_SIZE;
		bd_list[index].pbdev->bdif->ph_bcnt = bd_list[index].pSD->sectorCount();
	}
    return EOK;
}

//******************************************************************************
// Low level block (sector) read.
//******************************************************************************
static int ext4_bd_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
                          uint32_t blk_cnt) {
	int index;
	uint8_t status;

	index = get_bdev(bdev);
	if(index == -1)
		index = get_device_index(bdev);

	if(index <= 2) {
		status = bd_list[index].pDrive->msReadBlocks(blk_id,
								(uint32_t)blk_cnt, bdev->bdif->ph_bsize, buf);
		if (status != 0) return EIO;
	} else {
		status = bd_list[index].pSD->readSectors(blk_id,
								(uint8_t *)buf, blk_cnt);
		if (status == false)
			return EIO;
	}
	return EOK;
}
//******************************************************************************

//******************************************************************************
// Low level block (sector) write.
//******************************************************************************
static int ext4_bd_bwrite(struct ext4_blockdev *bdev, const void *buf,
			  uint64_t blk_id, uint32_t blk_cnt)
{
	int index;
	uint8_t status;

	index = get_bdev(bdev);
	if(index == -1)
		index = get_device_index(bdev);

	if(index <= 2) {
		status = bd_list[index].pDrive->msWriteBlocks(blk_id,
								(uint32_t)blk_cnt, bdev->bdif->ph_bsize, buf);
	if (status != 0) return EIO;
	} else {
		status = bd_list[index].pSD->writeSectors(blk_id,
								(uint8_t *)buf, blk_cnt);
		if (status == false)
			return EIO;
	}
	return EOK;
}
//******************************************************************************
// Low level block device close. (how to implement?).
//******************************************************************************
static int ext4_bd_close(struct ext4_blockdev *bdev)
{
	(void)bdev;
	return EOK;
}

//******************************************************************************
// Not implemeted yet. TODO.
//******************************************************************************
static int ext4_bd_ctrl(struct ext4_blockdev *bdev, int cmd, void *args)
{
	(void)bdev;
	return EOK;
}
//******************************************************************************

//******************************************************************************
// Get low level block device index. This is up to three USB devices.
//******************************************************************************
int get_bdev(struct ext4_blockdev * bdev) {
	int index;
	int ret = -1;
	for (index = 0; index < CONFIG_EXT4_BLOCKDEVS_COUNT; index++)
	{
		if (bdev == ext4_blkdev_list[index]) {
			ret = index;
			break;
		}
	}
	return ret;
}

//******************************************************************************
// Get block device (partition) index.
//******************************************************************************
int get_device_index(struct ext4_blockdev *bdev) {
	int index;
	int ret = -1;
	for (index = 0; index < MAX_MOUNT_POINTS; index++)
	{
		if (bdev == (struct ext4_blockdev *)&mount_list[index].partbdev) {
			if(mount_list[index].parent_bd.connected)
				ret = mount_list[index].parent_bd.dev_id;
				break;
		}
	}
	return ret;
}

//******************************************************************************
// Return pointer to mount list.
//******************************************************************************
bd_mounts_t * get_mount_list(void) {
	return &mount_list[0];
}

//******************************************************************************
// Helper function to attach mount name to filename. /sdxx/filename.
//******************************************************************************
void lwext_get_mp(const char *fn, uint8_t id) {
	char tbuf[256];
	sprintf(tbuf,"%s%s",mount_list[id].pname, fn);
	strcpy((char *)fn,tbuf);
}

//******************************************************************************
// Init block device.
//******************************************************************************
int LWextFS::init_block_device(uint8_t dev) {
	uint8_t status = -1;
	myusb.Task();

//	if(!bd_list[dev].pDrive->filesystemsStarted())
//		bd_list[dev].pDrive->startFilesystems(); 
	
	if(dev < CONFIG_EXT4_BLOCKDEVS_COUNT - 1) {
		device_list[dev]->begin();
		if(device_list[dev]) 
			status = device_list[dev]->checkConnectedInitialized();
			if(status == EOK) {
				bd_list[dev].pDrive = device_list[dev];
				bd_list[dev].dev_id = dev;
				bd_list[dev].connected = true;
				sprintf(bd_list[dev].name,"%s",deviceName[dev]);
				bd_list[dev].pbdev = ext4_blkdev_list[dev];
			} else if (status != EOK) {
				sprintf(bd_list[dev].name,"UnKnown");
				bd_list[dev].connected = false;
			}
	} else {
		sd = cardFactory.newCard(SD_CONFIG);
		if(sd && !sd->errorCode()) {
			bd_list[dev].pSD = sd;
			bd_list[dev].dev_id = dev;
			bd_list[dev].connected = true;
			sprintf(bd_list[dev].name,"%s",deviceName[dev]);
			bd_list[dev].pbdev = ext4_blkdev_list[dev];
		} else {
			sprintf(bd_list[dev].name,"UnKnown");
			bd_list[dev].connected = false;
		}
	}
	if(bd_list[dev].connected) {
		for(int i = 0; i < 4; i++) {
			mount_list[(dev*4)+i].parent_bd = bd_list[dev];
		}
		scan_mbr(dev); // Get all partition info for this device.
	}
	return EOK;
}

//******************************************************************************
// Initial setup and initilaization of block devices;
//******************************************************************************
int LWextFS::lwext_init_devices(void) {
	int availableDevices = 0;
	
	for(int i = 0; i < CONFIG_EXT4_BLOCKDEVS_COUNT; i++) {
		bd_list[i].connected = false;		
		init_block_device(i);
		if(bd_list[i].connected) {
			availableDevices++;
		}
	}
	if(availableDevices == 0)
		return -1;
	else
		return availableDevices;
}

//******************************************************************************
// Stat function returns file type regular file or directory and file size if
// regular file in buf. (stat_t struct)
//******************************************************************************
int lwext_stat(const char *filename, stat_t *buf) {
	int r = ENOENT;

	if(('\0' == TO_LWEXT_PATH(filename)[0])
		|| (0 == strcmp(TO_LWEXT_PATH(filename),"/")) ) {// just the root
		buf->st_mode = S_IFDIR;
		buf->st_size = 0;
		r = 0;
	} else {
		union {
			ext4_dir dir;
			ext4_file f;
		} var;
		r = ext4_dir_open(&(var.dir), TO_LWEXT_PATH(filename));
		if(0 == r) {
			(void) ext4_dir_close(&(var.dir));
			buf->st_mode = S_IFDIR;
			buf->st_size = 0;
		} else {
			r = ext4_fopen(&(var.f), TO_LWEXT_PATH(filename), "rb");
			if( 0 == r) {
				buf->st_mode = S_IFREG;
				buf->st_size = ext4_fsize(&(var.f));
				(void)ext4_fclose(&(var.f));
			}
		}
	}
	return r;
}
//******************************************************************************

//******************************************************************************
// Initialize a block device. (0 - 15) Mounting device if valid. (this will change)
//******************************************************************************
bool LWextFS::begin(uint8_t device) {
	if(!mount_list[device].available) return false;
	// Device 0-15.
	// lwext4 only supports 4 partitions at a time.
	if(lwext_mount(device) > 0) return false;
	id = device;
	if(ext4_mount_point_stats((const char *)mount_list[device].pname,&stats) != EOK) { 
		mount_list[device].mounted = false;
		return false;
	}
	return true;
}

//******************************************************************************
// Return volume name.
//******************************************************************************
const char * LWextFS::getMediaName() {
	return stats.volume_name;
}

//******************************************************************************
// Scan MBR on block device for partition info.
//******************************************************************************
bool LWextFS::scan_mbr(uint8_t dev) {
	int r;

	//Get partition list into mount_list.
	r = ext4_mbr_scan(bd_list[dev].pbdev, &bdevs);
	if (r != EOK) {
		Serial.printf("ext4_mbr_scan error %d\n",r);
		return false;
	}
	int i;
	for (i = 0; i < 4; i++) {
		if (!bdevs.partitions[i].bdif) {
			strcpy(mount_list[(dev*4)+i].pname, "UnKnown"); // UnKnown.
			mount_list[(dev*4)+i].available = false;
			continue;
		}
		sprintf(mount_list[(dev*4)+i].pname, "/mp/%s/",mpName[(dev*4)+i]); // Assign partition name sda1... from list.
		mount_list[(dev*4)+i].partbdev = bdevs.partitions[i]; // Store partition info in mount list.
		mount_list[(dev*4)+i].available = true;
	}
	return true;
}

//******************************************************************************
// Mount a partition. Called by begin or external call. Recovers file system errors.
//******************************************************************************
int LWextFS::lwext_mount(uint8_t dev) {
	int r;
	mount_list[dev].mounted = false;
	if(!mount_list[dev].available) return ENXIO;
	r = ext4_device_register(&mount_list[dev].partbdev, mount_list[dev].pname);
	if (r != EOK) {
		Serial.printf("ext4_device_register: rc = %d\n", r);
		return r;
	}
	r = ext4_mount(mount_list[dev].pname, mount_list[dev].pname, false);
	if (r != EOK) {
		Serial.printf("ext4_mount: rc = %d\n", r);
		(void)ext4_device_unregister(mount_list[dev].pname);
		return r;
	}
	r = ext4_recover(mount_list[dev].pname);
	if (r != EOK && r != ENOTSUP) {
		Serial.printf("ext4_recover: rc = %d\n", r);
		return r;
	}
// Journaling not working yet
//	r = ext4_journal_start(mount_list[dev].pname);
//	if (r != EOK) {
//		Serial.printf("ext4_journal_start: rc = %d\n", r);
//		return false;
//	}
	ext4_cache_write_back(mount_list[dev].pname, 1);
	mount_list[dev].mounted = true;
	return EOK;
}

//******************************************************************************
// Cleanly unmount partition. Needs to be called before removing device!!!
//******************************************************************************
bool LWextFS::lwext_umount(uint8_t dev) {
	int r;

	ext4_cache_write_back(mount_list[dev].pname, 0);
// Journaling not working yet
//	r = ext4_journal_stop(mount_list[dev].pname);
//	if (r != EOK) {
//		Serial.printf("ext4_journal_stop: fail %d", r);
//		return false;
//	}
	r = ext4_umount(mount_list[dev].pname);
	if (r != EOK) {
		Serial.printf("ext4_umount: fail %d", r);
		return false;
	}
	mount_list[dev].mounted = false;
	return true;
}


//******************************************************************************
// Returns mount status for a device in mpInfo struct.
//******************************************************************************
int LWextFS::getMountStats(const char *vol, struct ext4_mount_stats *mpInfo) {
	return ext4_mount_point_stats(vol, mpInfo);
}

