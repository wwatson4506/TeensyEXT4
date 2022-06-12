//  Teensy USB Drive lwext4 testing 
// Based on lwext4 by: Grzegorz Kostka (kostka.grzegorz@gmail.com)
// Modified by: Wareen Watson for the Teensy 4.x
   
#include "USBHost_t36.h"
#include "ext4.h"
#include "usb_bd.h"
#include "ext4_mbr.h"
#include <string.h>

// ******** These are changable for testing purposes *****************
/**@brief   Read-write size*/
static int rw_szie = 32 * 1024;

/**@brief   Read-write size*/
static int rw_count = 1000;

/**@brief   Directory test count*/
static int dir_cnt = 0;

/**@brief   Cleanup after test.*/
static bool cleanup_flag = false;

/**@brief   Block device stats.*/
static bool bstat = false;

/**@brief   Superblock stats.*/
static bool sbstat = false;

/**@brief   Verbose mode*/
static bool verbose = false;
//********************************************************************

// TODO: This needs to change.
extern 	USBHost myusb;

static struct ext4_bcache *bc;
static struct ext4_blockdev *bd;

struct ext4_io_stats {
	float io_read;
	float io_write;
	float cpu;
};

void io_timings_clear(void);
const struct ext4_io_stats *io_timings_get(uint32_t time_sum_ms);

// Directory entry types.
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

static long int get_ms(void) { return millis(); }

// Not used.
void io_timings_clear(void)
{
}
const struct ext4_io_stats *io_timings_get(uint32_t time_sum_ms)
{
	return NULL;
}

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

static void printf_io_timings(long int diff) {
	const struct ext4_io_stats *stats = io_timings_get(diff);
	if (!stats)
		return;

	printf("io_timings:\n");
	printf("  io_read: %.3f%%\n", (double)stats->io_read);
	printf("  io_write: %.3f%%\n", (double)stats->io_write);
	printf("  io_cpu: %.3f%%\n", (double)stats->cpu);
}

static bool mbr_scan(void) {
	int r;
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

bool test_lwext4_mount(struct ext4_blockdev *bdev, struct ext4_bcache *bcache) {
	int r;

	bc = bcache;
	bd = bdev;

	if (!bd) {
		Serial.printf("test_lwext4_mount: no block device\n");
		return false;
	}

  if (verbose)
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
	r = ext4_journal_start("/");
	if (r != EOK) {
		Serial.printf("ext4_journal_start: rc = %d\n", r);
		return false;
	}

	ext4_cache_write_back("/mp/", 1);
	return true;
}

bool test_lwext4_umount(void) {
	int r;

	ext4_cache_write_back("/mp/", 0);

	r = ext4_journal_stop("/mp/");
	if (r != EOK) {
		Serial.printf("ext4_journal_stop: fail %d", r);
		return false;
	}

	r = ext4_umount("/mp/");
	if (r != EOK) {
		Serial.printf("ext4_umount: fail %d", r);
		return false;
	}
	return true;
}

static int verify_buf(const unsigned char *b, size_t len, unsigned char c){
	size_t i;
	for (i = 0; i < len; ++i) {
		if (b[i] != c)
			return c - b[i];
	}

	return 0;
}

bool test_lwext4_file_test(uint8_t *rw_buff, uint32_t rw_size, uint32_t rw_count) {
	int r;
	size_t size;
	uint32_t i;
	long int start;
	long int stop;
	long int diff;
	uint32_t kbps;
	uint64_t size_bytes;

	ext4_file f;

	printf("file_test:\n");
	printf("  rw size: %" PRIu32 "\n", rw_size);
	printf("  rw count: %" PRIu32 "\n", rw_count);

	/*Add hello world file.*/
	r = ext4_fopen(&f, "/mp/hello.txt", "wb");
	r = ext4_fwrite(&f, "Hello World !\n", strlen("Hello World !\n"), 0);
	r = ext4_fclose(&f);

	io_timings_clear();
	start = get_ms();
	r = ext4_fopen(&f, "/mp/test1", "wb");
	if (r != EOK) {
		printf("ext4_fopen ERROR = %d\n", r);
		return false;
	}

	printf("ext4_write: %" PRIu32 " * %" PRIu32 " ...\n", rw_size,
	       rw_count);
	for (i = 0; i < rw_count; ++i) {

		memset(rw_buff, i % 10 + '0', rw_size);

		r = ext4_fwrite(&f, rw_buff, rw_size, &size);

		if ((r != EOK) || (size != rw_size))
			break;
	}

	if (i != rw_count) {
		printf("  file_test: rw_count = %" PRIu32 "\n", i);
		return false;
	}

	stop = get_ms();
	diff = stop - start;
	size_bytes = rw_size * rw_count;
	size_bytes = (size_bytes * 1000) / 1024;
	kbps = (size_bytes) / (diff + 1);
	printf("  write time: %d ms\n", (int)diff);
	printf("  write speed: %" PRIu32 " KB/s\n", kbps);
	printf_io_timings(diff);
	r = ext4_fclose(&f);

	io_timings_clear(); // Not used.
	start = get_ms();
	r = ext4_fopen(&f, "/mp/test1", "r+");
	if (r != EOK) {
		printf("ext4_fopen ERROR = %d\n", r);
		return false;
	}

	printf("ext4_read: %" PRIu32 " * %" PRIu32 " ...\n", rw_size, rw_count);

	for (i = 0; i < rw_count; ++i) {
		r = ext4_fread(&f, rw_buff, rw_size, &size);

		if ((r != EOK) || (size != rw_size))
			break;

		if (verify_buf(rw_buff, rw_size, i % 10 + '0'))
			break;
	}

	if (i != rw_count) {
		printf("  file_test: rw_count = %" PRIu32 "\n", i);
		return false;
	}

	stop = get_ms();
	diff = stop - start;
	size_bytes = rw_size * rw_count;
	size_bytes = (size_bytes * 1000) / 1024;
	kbps = (size_bytes) / (diff + 1);
	printf("  read time: %d ms\n", (int)diff);
	printf("  read speed: %d KB/s\n", (int)kbps);
	printf_io_timings(diff);

	r = ext4_fclose(&f);
	return true;
}

void test_lwext4_cleanup(void) {
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
	io_timings_clear();
	start = millis();
	r = ext4_dir_rm("/mp/dir1");
	if (r != EOK && r != ENOENT) {
		Serial.printf("ext4_fremove ext4_dir_rm: rc = %d\n", r);
	}
	stop = millis();
	diff = stop - start;
	Serial.printf("cleanup: time: %d ms\n", (uint32_t)diff);
}

void test_lwext4_mp_stats(void) {
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

void test_lwext4_dir_ls(const char *path) {
	char sss[256];
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
		Serial.printf("  %s%s\n", entry_to_str(de->inode_type), sss);
		de = ext4_dir_entry_next(&d);
	}
	ext4_dir_close(&d);
}

void test_lwext4_block_stats(void) {
	if (!bd)
		return;

	printf("********************\n");
	printf("ext4 blockdev stats\n");
	printf("bdev->bread_ctr = %" PRIu32 "\n", bd->bdif->bread_ctr);
	printf("bdev->bwrite_ctr = %" PRIu32 "\n", bd->bdif->bwrite_ctr);

	printf("bcache->ref_blocks = %" PRIu32 "\n", bd->bc->ref_blocks);
	printf("bcache->max_ref_blocks = %" PRIu32 "\n", bd->bc->max_ref_blocks);
	printf("bcache->lru_ctr = %" PRIu32 "\n", bd->bc->lru_ctr);

	printf("\n");
	printf("********************\n");
}

bool test_lwext4_dir_test(int len) {
	ext4_file f;
	int r;
	int i;
	char path[64];
	uint32_t diff;
	uint32_t stop;
	uint32_t start;

	Serial.printf("test_lwext4_dir_test: %d\n", len);
	start = millis();

	Serial.printf("directory create: /mp/dir1/\n");
	r = ext4_dir_mk("/mp/dir1/");
	if (r != EOK) {
		Serial.printf("ext4_dir_mk: rc = %d\n", r);
		return false;
	}

	Serial.printf("add files to: /mp/dir1/\n");
	for (i = 0; i < len; ++i) {
		sprintf(path, "/mp/dir1/f%d", i);
		r = ext4_fopen(&f, path, "wb");
		if (r != EOK) {
			Serial.printf("ext4_fopen: rc = %d\n", r);
			return false;
		}
	}
	stop = millis();
	diff = stop - start;
	test_lwext4_dir_ls("/mp/");
	Serial.printf("test_lwext4_dir_test: time: %d ms\n", (uint32_t)diff);
	Serial.printf("test_lwext4_dir_test: av: %d ms/entry\n", (uint32_t)diff / (len + 1));
	return true;
}

void setup() {
// Open serial communications and wait for port to open:
//   Serial.begin(9600);
   while (!Serial) {
    yield(); // wait for serial port to connect.
  }
  if(CrashReport)
	Serial.print(CrashReport);

  Serial.printf("%cTeensy EXT4 file system testing\n\n",12);

  myusb.begin();

  parent_blockdev = ext4_usb_bd_get();
//  Uncomment the line below and comment line above 
//  to test SDIO card.  
//  parent_blockdev = ext4_sd_bd_get();
  
  if (!parent_blockdev) {
	Serial.printf("open_blockdev: failed\n");
  } else {
	Serial.printf("open_blockdev: Passed\n");
  }
  if (verbose)
	ext4_dmask_set(DEBUG_ALL);

  if (!mbr_scan())
	Serial.printf("mbr_scan() Failed\n");
  else
	Serial.printf("mbr_scan() Passed\n\n");

  if (!test_lwext4_mount(&bdevs.partitions[0], bc))
	Serial.printf("test_lwext4_mount() Failed\n");
  else
	Serial.printf("test_lwext4_mount() Passed\n\n");

  test_lwext4_cleanup();

  if (sbstat)
    test_lwext4_mp_stats();

  test_lwext4_dir_ls("/mp/");

  test_lwext4_dir_test(dir_cnt);
  
  uint8_t *rw_buff = (uint8_t *)malloc(rw_szie);
  if (!rw_buff) {
    free(rw_buff);
	return EXIT_FAILURE;
  }
  if (!test_lwext4_file_test(rw_buff, rw_szie, rw_count)) {
	free(rw_buff);
	return EXIT_FAILURE;
  }
  free(rw_buff);

  test_lwext4_dir_ls("/mp/");
  
  if (sbstat)
    test_lwext4_mp_stats();	

  if (cleanup_flag)
	test_lwext4_cleanup();

  if (sbstat)
    test_lwext4_mp_stats();	

  if (bstat)
    test_lwext4_block_stats();

  test_lwext4_umount();

  Serial.printf("Done...\n");
}

void loop() {
}
