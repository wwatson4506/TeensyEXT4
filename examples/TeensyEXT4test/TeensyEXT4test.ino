//  MSC USB Drive testing 
   
#include "Arduino.h"
//#include "mscFS.h"
#include "USBHost_t36.h"
#include "ext4.h"
#include "blockdev.h"
#include "ext4_mbr.h"
#include "ext4MscHost.h"
#include <string.h>

extern void mscHostInit(void);
extern int mscInit(void);

static struct ext4_bcache *bc;
static struct ext4_blockdev *bd;

// A small hex dump function
void hexDump(const void *ptr, uint32_t len)
{
  uint32_t  i = 0, j = 0;
  uint8_t   c = 0;
  const uint8_t *p = (const uint8_t *)ptr;

  Serial.printf("BYTE      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
  Serial.printf("---------------------------------------------------------\n");
  for(i = 0; i <= (len-1); i+=16) {
   Serial.printf("%4.4x      ",i);
   for(j = 0; j < 16; j++) {
      c = p[i+j];
      Serial.printf("%2.2x ",c);
    }
    Serial.printf("  ");
    for(j = 0; j < 16; j++) {
      c = p[i+j];
      if(c > 31 && c < 127)
        Serial.printf("%c",c);
      else
        Serial.printf(".");
    }
    Serial.printf("\n");
  }

}

static const char *entry_to_str(uint8_t type)
{
	switch (type) {
	case EXT4_DE_UNKNOWN:
		return "[unk] ";
	case EXT4_DE_REG_FILE:
		return "[fil] ";
	case EXT4_DE_DIR:
		return "[dir] ";
	case EXT4_DE_CHRDEV:
		return "[cha] ";
	case EXT4_DE_BLKDEV:
		return "[blk] ";
	case EXT4_DE_FIFO:
		return "[fif] ";
	case EXT4_DE_SOCK:
		return "[soc] ";
	case EXT4_DE_SYMLINK:
		return "[sym] ";
	default:
		break;
	}
	return "[???]";
}

static struct ext4_blockdev *parent_blockdev;
struct ext4_blockdev *part_blockdev;
static struct ext4_mbr_bdevs bdevs;
static int dir_cnt = 1;

static bool mbr_scan(void)
{
	int r;
//	Serial.printf("ext4_mbr\n");
	r = ext4_mbr_scan(parent_blockdev, &bdevs);
	if (r != EOK) {
		Serial.printf("ext4_mbr_scan error %d\n",r);
		return false;
	}

	int i;
	Serial.printf("ext4_mbr_scan:\n");
	for (i = 0; i < 4; i++) {
		Serial.printf("mbr_entry %d:\n", i);
		if (!bdevs.partitions[i].bdif) {
			Serial.printf("\tempty/unknown\n");
			continue;
		}

		Serial.printf(" offeset: 0x%" PRIx64 ", %" PRIu64 "MB\n",
			bdevs.partitions[i].part_offset,
			bdevs.partitions[i].part_offset / (1024 * 1024));
		Serial.printf(" size:    0x%" PRIx64 ", %" PRIu64 "MB\n",
			bdevs.partitions[i].part_size,
			bdevs.partitions[i].part_size / (1024 * 1024));


	}

	return true;
}

bool test_lwext4_mount(struct ext4_blockdev *bdev, struct ext4_bcache *bcache)
{
	int r;

	bc = bcache;
	bd = bdev;

	if (!bd) {
		Serial.printf("test_lwext4_mount: no block device\n");
		return false;
	}

	ext4_dmask_set(DEBUG_ALL);

	r = ext4_device_register(bd, "ext4_fs");
	if (r != EOK) {
		Serial.printf("ext4_device_register: rc = %d\n", r);
		return false;
	}

	r = ext4_mount("ext4_fs", "/mp/", false);
	if (r != EOK) {
		Serial.printf("ext4_mount: rc = %d\n", r);
		return false;
	}


	r = ext4_recover("/mp/");
	if (r != EOK && r != ENOTSUP) {
		Serial.printf("ext4_recover: rc = %d\n", r);
		return false;
	}

// Journaling not working yet
//	r = ext4_journal_start("/mp/");
//	if (r != EOK) {
//		Serial.printf("ext4_journal_start: rc = %d\n", r);
//		return false;
//	}

	ext4_cache_write_back("/mp/", 1);
	return true;
}

void test_lwext4_cleanup(void)
{
	uint32_t start;
	uint32_t stop;
	uint32_t diff;
	int r;

	Serial.printf("\ncleanup:\n");
	r = ext4_fremove("/mp/hello.txt");
	if (r != EOK && r != ENOENT) {
		Serial.printf("ext4_fremove error: rc = %d\n", r);
	}

	Serial.printf("remove /mp/test1\n");
	r = ext4_fremove("/mp/test1");
	if (r != EOK && r != ENOENT) {
		Serial.printf("ext4_fremove error: rc = %d\n", r);
	}

	Serial.printf("remove /mp/dir1\n");
//	io_timings_clear();
	start = millis();
	r = ext4_dir_rm("/mp/dir1");
	if (r != EOK && r != ENOENT) {
		Serial.printf("ext4_fremove ext4_dir_rm: rc = %d\n", r);
	}
	stop = millis();
	diff = stop - start;
	Serial.printf("cleanup: time: %d ms\n", (uint32_t)diff);
//	printf_io_timings(diff);
}

void test_lwext4_mp_stats(void)
{
	struct ext4_mount_stats stats;
	ext4_mount_point_stats("/mp/", &stats);

	Serial.printf("********************\n");
	Serial.printf("ext4_mount_point_stats\n");
	Serial.printf("inodes_count = %" PRIu32 "\n", stats.inodes_count);
	Serial.printf("free_inodes_count = %" PRIu32 "\n", stats.free_inodes_count);
	Serial.printf("blocks_count = %" PRIu32 "\n", (uint32_t)stats.blocks_count);
	Serial.printf("free_blocks_count = %" PRIu32 "\n",
	       (uint32_t)stats.free_blocks_count);
	Serial.printf("block_size = %" PRIu32 "\n", stats.block_size);
	Serial.printf("block_group_count = %" PRIu32 "\n", stats.block_group_count);
	Serial.printf("blocks_per_group= %" PRIu32 "\n", stats.blocks_per_group);
	Serial.printf("inodes_per_group = %" PRIu32 "\n", stats.inodes_per_group);
	Serial.printf("volume_name = %s\n", stats.volume_name);
	Serial.printf("********************\n");
}

void test_lwext4_dir_ls(const char *path)
{
	char sss[255];
	ext4_dir d;
	const ext4_direntry *de;

	Serial.printf("ls %s\n", path);

	int r = ext4_dir_open(&d, path);
	if(r != EOK)
	  Serial.printf("ext4_dir_open(): Failed %d\n",r);
	else
	  Serial.printf("ext4_dir_open(): Passed\n");
	
	de = ext4_dir_entry_next(&d);

	while (de) {
		memcpy(sss, de->name, de->name_length);
		sss[de->name_length] = 0;
		Serial.printf("  %s\n", sss);
//		Serial.printf("  %s%s\n", entry_to_str(de->inode_type), sss);
		de = ext4_dir_entry_next(&d);
	}
	ext4_dir_close(&d);
}

bool test_lwext4_dir_test(int len)
{
	ext4_file f;
	int r;
	int i;
	char path[64];
	uint32_t diff;
	uint32_t stop;
	uint32_t start;

	Serial.printf("test_lwext4_dir_test: %d\n", len);
	start = millis();
/*
	Serial.printf("directory create: /mp/dir1\n");
	r = ext4_dir_mk("/mp/dir1");
	if (r != EOK) {
		Serial.printf("ext4_dir_mk: rc = %d\n", r);
		return false;
	}
/*
	Serial.printf("add files to: /mp/dir1\n");
	for (i = 0; i < len; ++i) {
		sprintf(path, "/mp/dir1/f%d", i);
		r = ext4_fopen(&f, path, "wb");
		if (r != EOK) {
			Serial.printf("ext4_fopen: rc = %d\n", r);
			return false;
		}
	}
*/
	stop = millis();
	diff = stop - start;
	test_lwext4_dir_ls("/mp/");
	Serial.printf("test_lwext4_dir_test: time: %d ms\n", (uint32_t)diff);
	Serial.printf("test_lwext4_dir_test: av: %d ms/entry\n", (uint32_t)diff / (len + 1));
	return true;
}

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    yield(); // wait for serial port to connect.
  }
  Serial.printf("Teensy EXT4 file system testing\n\n");

  mscHostInit();  // Initialize USBHost

  parent_blockdev = ext4_blockdev_get();
  if (!parent_blockdev) {
	Serial.printf("open_filedev: failed\n");
  } else {
	Serial.printf("open_filedev: Passed\n");
  }

  if (!mbr_scan())
	Serial.printf("mbr_scan() Failed\n");
  else
	Serial.printf("mbr_scan() Passed\n\n");

  if (!test_lwext4_mount(&bdevs.partitions[0], bc))
	Serial.printf("test_lwext4_mount() Failed\n");
  else
	Serial.printf("test_lwext4_mount() Passed\n\n");

  test_lwext4_mp_stats();	

  test_lwext4_cleanup();

//  if (!test_lwext4_dir_test(dir_cnt))
//	Serial.printf("test_lwext4_dir_test(dir_cnt): Failed\n");
//  else
//	Serial.printf("test_lwext4_dir_test(dir_cnt): Passed\n");


  Serial.printf("Done...\n");
}

void loop() {
}
