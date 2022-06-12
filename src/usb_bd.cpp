/*
 * Copyright (c) 2015 Grzegorz Kostka (kostka.grzegorz@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "Arduino.h"
#include "ext4_config.h"
#include "ext4_blockdev.h"
#include "usb_bd.h"
#include "ext4_errno.h"
#include "USBHost_t36.h"

// Setup USBHost_t36 and as many HUB ports as needed.
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);

// Instances for the number of USB drives you are using.
USBDrive myDrive1(myusb);
USBDrive myDrive2(myusb);

/**@brief   Block size.*/
#define USB_MSC_BLOCK_SIZE 512

/**@brief   MBR_block ID*/
#define MBR_BLOCK_ID 0
#define MBR_PART_TABLE_OFF 446

struct part_tab_entry {
	uint8_t status;
	uint8_t chs1[3];
	uint8_t type;
	uint8_t chs2[3];
	uint32_t first_lba;
	uint32_t sectors;
} __attribute__((packed));

/**@brief   Partition block offset*/
static uint32_t part_offset;


/**********************BLOCKDEV INTERFACE**************************************/
static int usb_bd_open(struct ext4_blockdev *bdev);
static int usb_bd_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
			 uint32_t blk_cnt);
static int usb_bd_bwrite(struct ext4_blockdev *bdev, const void *buf,
			  uint64_t blk_id, uint32_t blk_cnt);
static int usb_bd_close(struct ext4_blockdev *bdev);
static int usb_bd_lock(struct ext4_blockdev *bdev);
static int usb_bd_unlock(struct ext4_blockdev *bdev);

/******************************************************************************/
EXT4_BLOCKDEV_STATIC_INSTANCE(_usb_bd, 512, 0, usb_bd_open,
			      usb_bd_bread, usb_bd_bwrite, usb_bd_close,
			      0, 0);

/******************************************************************************/
static int usb_bd_open(struct ext4_blockdev *bdev)
{
	(void)bdev;
	myDrive1.startFilesystems();
	if(!myDrive1.filesystemsStarted()) {
		return EIO;
	} else {
		_usb_bd.part_offset = 0;
		_usb_bd.part_size = myDrive1.msDriveInfo.capacity.Blocks * myDrive1.msDriveInfo.capacity.BlockSize;
		_usb_bd.bdif->ph_bcnt = myDrive1.msDriveInfo.capacity.Blocks;
	}
    return EOK;
}

/******************************************************************************/

static int usb_bd_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
                          uint32_t blk_cnt)
{
	uint8_t status;

	status = myDrive1.msReadBlocks(blk_id + part_offset, (uint32_t)blk_cnt,
	                               bdev->bdif->ph_bsize, buf);

	if (status != 0)
		return EIO;
	return EOK;
}


/******************************************************************************/
static int usb_bd_bwrite(struct ext4_blockdev *bdev, const void *buf,
			  uint64_t blk_id, uint32_t blk_cnt)
{
	uint8_t status;
	status = myDrive1.msWriteBlocks(blk_id + part_offset, (uint32_t)blk_cnt,  bdev->bdif->ph_bsize, buf);

	if (status != 0)
		return EIO;
	return EOK;
}
/******************************************************************************/
static int usb_bd_close(struct ext4_blockdev *bdev)
{
	(void)bdev;
	return EOK;
}

/******************************************************************************/
struct ext4_blockdev *ext4_usb_bd_get(void)
{
	return &_usb_bd;
}
/******************************************************************************/

extern "C" {
__attribute__((weak)) 
int _write(int file, char *ptr, int len)
{
	int i;
	for(i = 0; i < len; i++)
	{
		usb_serial_putchar(*(ptr+i));
		if (*(ptr+i) == '\n')
			usb_serial_putchar('\r');
   }
	usb_serial_flush_output();
	return len;
}
}

