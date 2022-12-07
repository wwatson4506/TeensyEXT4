/* ext4fs library compatibility wrapper for use of lwext on Teensy
 * Copyright (c) 2022, Warren Watson.
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

#ifndef __EXT4FS_H__
#define __EXT4FS_H__

#include "USBHost_t36.h"

#ifdef __cplusplus
  extern "C" {
#endif
#include "ext4/ext4_config.h"
#include "ext4/ext4_mbr.h"
#include "ext4/ext4.h"
#include "ext4/ext4_fs.h"
#include "ext4/ext4_mkfs.h"
#ifdef __cplusplus
}
#endif

// Uncomment USE_RW_LEDS below to show read/write activity. LED pins are defined
// in ext4FS.h.
#define USE_RW_LEDS 1

// Setup debugging LED pin defs. (Uncomment
// Used mainly to see the activity of reads and writes.
#define WRITE_PIN			33		// Pin number of drive write activity led (RED LED).
#define READ_PIN			34		// Pin number of drive read activity led (GREEN LED).

// Ext2/3/4 File system type in MBR.
#define EXT4_TYPE 0x83
#define EXT4_BLOCK_SIZE 512

enum {USB_TYPE=0, SD_TYPE=CONFIG_EXT4_BLOCKDEVS_COUNT-1}; // what type of block device

/* controls for block devices */
#define DEVICE_CTRL_GET_SECTOR_SIZE  0
#define DEVICE_CTRL_GET_BLOCK_SIZE   1
#define DEVICE_CTRL_GET_SECTOR_COUNT 2
#define DEVICE_CTRL_GET_DISK_SIZE    3

#define TO_LWEXT_PATH(f) (&((f)[0]))

#define FMR "r+"
#define FMWC "w+"
#define FMWA "a+"

#define ROOT_DIR "/" 

// only 4 mount points allowed at this time.
#define MAX_MOUNT_POINTS 16
// Low level device names
const char deviceName[][4] = {"sda","sdb","sdc","sdd"};
// Partition device names.
const char mpName[][MAX_MOUNT_POINTS] = 
{	"sda1",
	"sda2",
	"sda3",
	"sda4",
	"sdb1",
	"sdb2",
	"sdb3",
	"sdb4",
	"sdc1",
	"sdc2",
	"sdc3",
	"sdc4",
	"sdd1",
	"sdd2",
	"sdd3",
	"sdd4"
};

//******************************************************************************
// The following structures used for low level block devices and mount info.
//******************************************************************************
// Physical Block Device.
typedef struct block_device {
	int dev_id = -1;
	char name[32];
	USBDrive *pDrive;
	SdCard   *pSD;
	struct ext4_blockdev *pbdev; // &_ext4_bd ...&_ext4_bd3
	bool connected = false;
}block_device_t;

//  Mount Point Info.
typedef struct bd_mounts {
	bool available = false;
	char volName[32];
	char pname[32];
	struct ext4_blockdev partbdev;
    uint8_t pt = 0;
	block_device_t parent_bd;
	bool mounted = false;
}bd_mounts_t;

DMAMEM static block_device_t bd_list[CONFIG_EXT4_BLOCKDEVS_COUNT];
DMAMEM static bd_mounts_t mount_list[MAX_MOUNT_POINTS];

// Stat struct. File type and size.
typedef struct {
	uint32_t st_mode;     // File mode
	size_t   st_size;     // File size (regular files only)
} stat_t;

//******************************************************************************
// Non-ext4FS protos.
//******************************************************************************
#ifdef __cplusplus
extern "C" {
#endif

int get_bdev(struct ext4_blockdev * bdev);
int get_device_index(struct ext4_blockdev *bdev);
void init_device_id(void);
void mkfsPrint(const char *ch);
#ifdef __cplusplus
}
#endif
//******************************************************************************

class ext4FS;

// Use FILE_READ & FILE_WRITE as defined by FS.h
#if defined(FILE_READ) && !defined(FS_H)
#undef FILE_READ
#endif
#if defined(FILE_WRITE) && !defined(FS_H)
#undef FILE_WRITE
#endif
#include <FS.h>

//#define EXT4_FILE ext4_file

class EXT4File : public FileImpl
{
private:
	// Classes derived from FileImpl are never meant to be constructed from
	// anywhere other than openNextFile() and open() in their parent FS
	// class.  Only the abstract File class which references these
	// derived classes is meant to have a public constructor!
	EXT4File(ext4_file *filein, const char *name) {
		file = filein;
		dir = nullptr;
		strlcpy(fullpath, name, sizeof(fullpath));
	}
	EXT4File(ext4_dir *dirin, const char *name) {
		dir = dirin;
		file = nullptr;
		strlcpy(fullpath, name, sizeof(fullpath));
	}
	friend class ext4FS;
public:
	virtual ~EXT4File(void) {
		close();
	}

	// These will all return false as only some FS support it.

  	virtual bool getCreateTime(DateTimeFields &tm){
		uint32_t mdt = getCreationTime();
		if (mdt == 0) { return false;} // did not retrieve a date;
		breakTime(mdt, tm);
		return true;  	}
  	virtual bool getModifyTime(DateTimeFields &tm){
		uint32_t mdt = getModifiedTime();
		if (mdt == 0) {return false;} // did not retrieve a date;
		breakTime(mdt, tm);
		return true;
  	}
	virtual bool setCreateTime(const DateTimeFields &tm) {
		if (tm.year < 80 || tm.year > 207) return false;
		bool success = true;
		uint32_t mdt = makeTime(tm);
		int rcode = ext4_ctime_set(name(), (uint32_t) mdt);
		if(rcode != EOK)
			success = false;
		return success;
	}
	virtual bool setModifyTime(const DateTimeFields &tm) {
		if (tm.year < 80 || tm.year > 207) return false;
		bool success = true;
		uint32_t mdt = makeTime(tm);
		int rcode = ext4_mtime_set(name(), (uint32_t) mdt);
		if(rcode != EOK) 
			success = false;
		return success;
	}
	virtual size_t write(const void *buf, size_t size) {
		if (!file) return 0;
		size_t bw = 0;
		if(ext4_fwrite(file, buf, size, &bw) != EOK)
			return -1;
		else
		    return bw;
	}
	virtual int peek() {
		return -1; // TODO...
	}
	virtual int available() {
		if (!file) return 0;
		int64_t pos = ext4_ftell(file);
		if (pos < 0) return 0;
		int64_t size = ext4_fsize(file);
		if (size < 0) return 0;
		return size - pos;
	}
	virtual void flush() {
		ext4_cache_flush(fullpath);
	}
	virtual size_t read(void *buf, size_t nbyte) {
		if (file) {
			size_t br = 0;
			if(ext4_fread(file, buf, nbyte, &br) != EOK)
				return -1;
			else
				return br;
		}
		return 0;
	}
	virtual bool truncate(uint64_t size=0) {
		if (!file) return false;
		if (ext4_ftruncate(file, size) == EOK) return true;
		return false;
	}
	virtual bool seek(uint64_t pos, int mode = SeekSet) {
		if (!file) return false;
		int whence;
		if (mode == SeekSet) whence = SEEK_SET;
		else if (mode == SeekCur) whence = SEEK_CUR;
		else if (mode == SeekEnd) whence = SEEK_END;
		else return false;
		if (ext4_fseek(file, pos, whence) >= 0) return true;
		return false;
	}
	virtual uint64_t position() {
		if (!file) return 0;
		int64_t pos = ext4_ftell(file);
		if (pos < 0) pos = 0;
		return pos;
	}
	virtual uint64_t size() {
		if (!file) return 0;
		int64_t size = ext4_fsize(file);
		if (size < 0) size = 0;
		return size;
	}
	virtual void close() {
		if (file) {
			ext4_fclose(file);
			file = nullptr;
		}
		if (dir) {
			ext4_dir_close(dir);
			dir = nullptr;
		}
	}
	virtual bool isOpen() {
		return file || dir;
	}
	virtual const char * name() {
		const char *p = strrchr(fullpath, '/');
		if (p) return p + 1;
		return fullpath;
	}
	virtual boolean isDirectory(void) {
		return dir != nullptr;
	}
	virtual File openNextFile(uint8_t mode=0) {
		if (!dir) return File();
		do {
			if ((de = ext4_dir_entry_next(dir)) == NULL) return File();
		} while (strcmp((const char *)de->name, ".") == 0 || strcmp((const char *)de->name, "..") == 0);
		char pathname[128];
		strlcpy(pathname, fullpath, sizeof(pathname));
		size_t len = strlen(pathname);
		if (len > 0 && pathname[len-1] != '/' && len < sizeof(pathname)-2) {
			// add trailing '/', if not already present
			pathname[len++] = '/';
			pathname[len] = 0;
		}
		strlcpy(pathname + len, (const char *)de->name, sizeof(pathname) - len);
		if (de->inode_type == EXT4_DE_REG_FILE) {
			ext4_file *f = (ext4_file *)malloc(sizeof(ext4_file));
			if (!f) return File();
			if (ext4_fopen(f, pathname, "rb") == EOK) {
				return File(new EXT4File(f, pathname));
			}
			free(f);
		} else { // DIR
			ext4_dir *d = (ext4_dir *)malloc(sizeof(ext4_dir));
			if (!d) return File();
			if (ext4_dir_open(d, pathname) == EOK) {
				return File(new EXT4File(d, pathname));
			}
			free(d);
		}
		return File();
	}
	virtual void rewindDirectory(void) {
		if (dir) ext4_dir_entry_rewind(dir);
	}
private:
	ext4_file *file;
	ext4_dir *dir;
	const ext4_direntry *de;
	char *filename;
	char fullpath[256];

	uint32_t getCreationTime() {
		uint32_t filetime = 0;
		int rc = ext4_ctime_get(fullpath, (uint32_t *)&filetime);
		if(rc != sizeof(filetime))
			filetime = 0;   // Error so clear read value		
		return filetime;
	}
	uint32_t getModifiedTime() {
		uint32_t filetime = 0;
		int rc = ext4_mtime_get(fullpath, (uint32_t *)&filetime);
		if(rc != EOK) 
			filetime = 0;   // Error so clear read value	
		return filetime;
	}
};

class ext4FS : public FS
{
public:
	ext4FS() { init(); }
	~ext4FS() {}
	
	virtual bool format(int type=0, char progressChar=0, Print& pr=Serial) {
		return true;
	}
	void init();
	bool clr_BDL_entry(uint8_t dev);
	int init_block_device(void *drv, uint8_t dev);
	bool scan_mbr(uint8_t dev);
	bool begin(uint8_t device);
	const char * getMediaName();
	int lwext_mount(uint8_t dev);
	bool lwext_umount(uint8_t dev);
	int getMountStats(const char * vol, struct ext4_mount_stats *mpInfo);
	int lwext_mkfs (struct ext4_blockdev *bdev, const char *label = "");
	const char *get_mp_name(uint8_t id);
	block_device_t *get_bd_list(void);
	bd_mounts_t *get_mount_list(void);
	int lwext_stat(const char *filename, stat_t *buf);
	const char * getVolumeLabel();
	void dumpBDList(void);
	void dumpMountList();

	File open(const char *filepath, uint8_t mode = FILE_READ) {
		if(strlen(filepath) == 1) filepath = ""; // Remove leading "/" for root dir.
		if(filepath[0] == '/') // Strip off leading '/' including '\0'
			memmove((char *)filepath,filepath+1, strlen(filepath));
		sprintf(fullPath,"%s%s",get_mp_name(id), filepath);
		int rcode = 0;
		if (mode == FILE_READ) {
			stat_t info;
			if (lwext_stat(fullPath, &info) != EOK) return File();
			if (info.st_mode == S_IFREG) {
				ext4_file *file = (ext4_file *)malloc(sizeof(ext4_file));
				if (!file) return File();
				if (ext4_fopen(file, fullPath, FMR) == EOK) {
					return File(new EXT4File(file, fullPath));
				}
				free(file);
			} else { // DIR
				ext4_dir *dir = (ext4_dir *)malloc(sizeof(ext4_dir));
				if (!dir) return File();
				if (ext4_dir_open(dir, fullPath) == EOK) {
					return File(new EXT4File(dir, fullPath));
				}
				free(dir);
			}
		} else {
			ext4_file *file = (ext4_file *)malloc(sizeof(ext4_file));
			if (!file) return File();
			if(mode == FILE_WRITE) {
				rcode = ext4_fopen(file, fullPath, FMWA);
			} else if (mode == FILE_WRITE_BEGIN) {
				rcode = ext4_fopen(file, fullPath, FMWC);
			}
			if(rcode == EOK) {
				//attributes get written when the file is closed
				uint32_t filetime = 0;
				uint32_t _now = Teensy3Clock.get();
				rcode = ext4_ctime_get(fullPath, (uint32_t *)&filetime);
				if(rcode != sizeof(filetime)) {
					rcode = ext4_ctime_set(fullPath, (uint32_t) _now);
					if(rcode != EOK)
						Serial.println("FO:: set attribute creation failed");
				}
				rcode = ext4_mtime_set(fullPath, (uint32_t ) _now);
				if(rcode != EOK)
					Serial.println("FO:: set attribute modified failed");
				return File(new EXT4File(file, fullPath));
			}
		}
		fullPath[0] = 0;
		return File();
	}
	bool exists(const char *filepath) {
		if(filepath[0] == '/') // Strip off leading '/' including '\0'
			memmove((char *)filepath,filepath+1, strlen(filepath));
		sprintf(fullPath,"%s%s",get_mp_name(id), filepath);
		stat_t info;
		if (lwext_stat(fullPath, &info) != EOK) return false;
		fullPath[0] = 0;
		return true;
	}
	bool mkdir(const char *filepath) {
		int rcode;
		if(filepath[0] == '/') // Strip off leading '/' including '\0'
			memmove((char *)filepath,filepath+1, strlen(filepath));
		sprintf(fullPath,"%s%s",get_mp_name(id), filepath);
		if (ext4_dir_mk(fullPath) != EOK) return false;
		uint32_t _now = Teensy3Clock.get();
		rcode = ext4_ctime_set(fullPath, (uint32_t) _now);
		if(rcode != EOK)
			Serial.println("Set creation time failed");
		rcode = ext4_mtime_set(fullPath, (uint32_t ) _now);
		if(rcode != EOK)
			Serial.println("Set modified time failed");
		ext4_cache_flush(fullPath);
		fullPath[0] = 0;
		return true;
	}
	bool rename(const char *oldfilepath, const char *newfilepath) {
		if(oldfilepath[0] == '/') // Strip off leading '/' including '\0'
			memmove((char *)oldfilepath,oldfilepath+1, strlen(oldfilepath));
		if(newfilepath[0] == '/') // Strip off leading '/' including '\0'
			memmove((char *)newfilepath,newfilepath+1, strlen(newfilepath));
		sprintf(fullPath,"%s%s",get_mp_name(id), oldfilepath);
		sprintf(fullPath1,"%s%s",get_mp_name(id), newfilepath);
		int rcode = ext4_frename(fullPath, fullPath1);
		if(rcode != EOK) {
			Serial.printf("rename() Failed: %d\n",rcode);
			 return false;
		}
		uint32_t _now = Teensy3Clock.get();
		rcode = ext4_mtime_set(fullPath1, (uint32_t ) _now);
		if(rcode != EOK)
			Serial.println("Set modified time failed");
		ext4_cache_flush(fullPath1);
		fullPath[0] = 0;
		fullPath1[0] = 0;
		return true;
	}
	bool remove(const char *filepath) {
		if(filepath[0] == '/') // Strip off leading '/' including '\0'
			memmove((char *)filepath,filepath+1, strlen(filepath));
		sprintf(fullPath,"%s%s",get_mp_name(id), filepath);
		if (ext4_fremove(fullPath) != EOK) return false;
		ext4_cache_flush(fullPath);
		fullPath[0] = 0;
		return true;
	}
	bool rmdir(const char *filepath) {
		if(filepath[0] == '/') // Strip off leading '/' including '\0'
			memmove((char *)filepath,filepath+1, strlen(filepath));
		sprintf(fullPath,"%s%s",get_mp_name(id), filepath);
		if(ext4_dir_rm(fullPath) != EOK) {
			fullPath[0] = 0;
			return false;
		}
		ext4_cache_flush(fullPath);
		fullPath[0] = 0;
		return true;
	}
	uint64_t usedSize() {
		sprintf(fullPath,"%s%s",get_mp_name(id), mpName[id]);
		if(ext4_mount_point_stats(fullPath, &stats) != EOK) return 0;
		fullPath[0] = 0;
		uint64_t blocks = stats.free_blocks_count;
		if (blocks <= 0 || (uint64_t)blocks > stats.blocks_count) return totalSize();
		return (stats.blocks_count - blocks) * stats.block_size;
	}
	uint64_t totalSize() {
		return stats.blocks_count * stats.block_size;
	}
	
protected:
	uint8_t id = 0;
	char fullPath[512];
	char fullPath1[512];
	struct ext4_mount_stats stats;
	struct ext4_mbr_bdevs bdevs;
};

extern ext4FS EXT;

#endif

