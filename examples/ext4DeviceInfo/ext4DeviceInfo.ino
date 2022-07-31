// Print ext4 volume info.

#include "ext4FS.h"

extern USBHost myusb;

LWextFS myext4fs;
LWextFS myext4fsp[16];

struct ext4_mount_stats stats;

// Grab a pointer to the mount list.
bd_mounts_t *ml = get_mount_list();

void setup() {
  int i = 0;  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) { ; // wait for Arduino Serial Monitor
  }
   while (!Serial) {
    yield(); // wait for serial port to connect.
  }
  if(CrashReport)
	Serial.print(CrashReport);

  Serial.printf("%cTeensy lwext device info\n\n",12);
  myusb.begin();

//ext4_dmask_set(DEBUG_ALL);

  Serial.println("Initializing all availble lwext devices.\n");
  Serial.println("Please Wait...\n");
  int devcount = myext4fs.lwext_init_devices();  
  Serial.print(devcount,DEC);
  Serial.println(" lwext devices found.\n\n");

  Serial.println("Now we will list all physical block devices.\n");
  waitforInput();  
  dumpBDList();

  Serial.println("Now we will list all mountable partitions.\n");
  waitforInput();  
  dumpMountList();

  Serial.println("Mount all mountable partitions (up to 4).\n");
  waitforInput();
  Serial.println("This can take some time, Please wait...");

  int partCount = 0;
  for(i = 0; i < MAX_MOUNT_POINTS; i++) {
    if(myext4fsp[i].begin(i) == true) {
      Serial.printf("Mounting partition %d: %s\n",i, ml[i].pname); 
      partCount++;
    }
    if(partCount > 3) break;
  }
  Serial.printf("\nFinished Mounting %d partitions.\n", partCount);

  Serial.println("\nNow we will show partition info for each mounted partition.\n");
  waitforInput();  

  for(i = 0; i < MAX_MOUNT_POINTS; i++) {
	if(ml[i].mounted) {
      Serial.printf("********************\n");
      Serial.printf("         Partition: %s\n", ml[i].pname);
      Serial.printf("           offeset: 0x%" PRIx64 ", %" PRIu64 "MB\n",
                                    ml[i].partbdev.part_offset,
                   ml[i].partbdev.part_offset / (1024 * 1024));
      Serial.printf("              size: 0x%" PRIx64 ", %" PRIu64 "MB\n",
                                      ml[i].partbdev.part_size,
                     ml[i].partbdev.part_size / (1024 * 1024));
      Serial.printf("Logical Block Size: %d\n", ml[i].partbdev.lg_bsize);
      Serial.printf("       Block count: %" PRIu64 "\n", ml[i].partbdev.lg_bcnt);
      Serial.printf("           FS type: 0x%x\n", ml[i].pt);
      myext4fsp[i].getMountStats(ml[i].pname, &stats);
      Serial.printf("********************\n");
      Serial.printf("ext4_mount_point_stats\n");
      Serial.printf("     inodes_count = %" PRIu32 "\n", stats.inodes_count);
      Serial.printf("free_inodes_count = %" PRIu32 "\n", stats.free_inodes_count);
      Serial.printf("     blocks_count = %" PRIu32 "\n", (uint32_t)stats.blocks_count);
      Serial.printf("free_blocks_count = %" PRIu32 "\n",
	       (uint32_t)stats.free_blocks_count);
      Serial.printf("       block_size = %" PRIu32 "\n", stats.block_size);
      Serial.printf("block_group_count = %" PRIu32 "\n", stats.block_group_count);
      Serial.printf(" blocks_per_group = %" PRIu32 "\n", stats.blocks_per_group);
      Serial.printf(" inodes_per_group = %" PRIu32 "\n", stats.inodes_per_group);
      Serial.printf("      volume_name = %s\n", stats.volume_name);
      Serial.printf("********************\n\n");
    }  

  }

  Serial.println("Unmounting All Mounted Drives\n");
  for(i = 0; i < MAX_MOUNT_POINTS; i++) {
    if(ml[i].mounted)
      myext4fsp[i].lwext_umount(i);
  }
 
  Serial.println("Done...");
}

void loop() {
}

void waitforInput()
{
  Serial.println("Press any key to continue\n");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
