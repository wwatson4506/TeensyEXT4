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
#include "sd_bd.h"
#include "ext4_errno.h"
#include "SdFat.h"

/**@brief   Block size.*/
#define SD_BLOCK_SIZE 512

/**@brief   MBR_block ID*/
#define MBR_BLOCK_ID 0
#define MBR_PART_TABLE_OFF 446

// SdFat usage with an SD card.
#define SD_CONFIG SdioConfig(FIFO_SDIO)
SdCardFactory cardFactory;
SdCard *sd;

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
static int sd_bd_open(struct ext4_blockdev *bdev);
static int sd_bd_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
			 uint32_t blk_cnt);
static int sd_bd_bwrite(struct ext4_blockdev *bdev, const void *buf,
			  uint64_t blk_id, uint32_t blk_cnt);
static int sd_bd_close(struct ext4_blockdev *bdev);
static int sd_bd_lock(struct ext4_blockdev *bdev);
static int sd_bd_unlock(struct ext4_blockdev *bdev);

/******************************************************************************/
EXT4_BLOCKDEV_STATIC_INSTANCE(_sd_bd, 512, 0, sd_bd_open,
			      sd_bd_bread, sd_bd_bwrite, sd_bd_close,
			      0, 0);

/******************************************************************************/
static int sd_bd_open(struct ext4_blockdev *bdev)
{
	(void)bdev;
	sd = cardFactory.newCard(SD_CONFIG);
	if (!sd || sd->errorCode()) {
		return EIO;
	} else {
		_sd_bd.part_offset = 0;
		_sd_bd.part_size = sd->sectorCount() * SD_BLOCK_SIZE;
		_sd_bd.bdif->ph_bcnt = sd->sectorCount();
	}
    return EOK;
}

/******************************************************************************/

static int sd_bd_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
                          uint32_t blk_cnt)
{
	bool status;
	
	status = sd->readSectors(blk_id + part_offset, (uint8_t *)buf, blk_cnt);
	if (status == false)
		return EIO;
	return EOK;
}


/******************************************************************************/
static int sd_bd_bwrite(struct ext4_blockdev *bdev, const void *buf,
			  uint64_t blk_id, uint32_t blk_cnt)
{
	bool status;

	status = sd->writeSectors(blk_id + part_offset, (const uint8_t*)buf, blk_cnt);
	if (status == false)
		return EIO;
	return EOK;
}
/******************************************************************************/
static int sd_bd_close(struct ext4_blockdev *bdev)
{
	(void)bdev;
	return EOK;
}

/******************************************************************************/
struct ext4_blockdev *ext4_sd_bd_get(void)
{
	return &_sd_bd;
}
/******************************************************************************/
