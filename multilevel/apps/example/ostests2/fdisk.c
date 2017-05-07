#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[FDISK_NUM_INODES];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int filesystemblocksize = 0;
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)
int num_filesystem_blocks = 0;
int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk


void main (int argc, char *argv[]){
  fdisk();
}


void fdisk ()
{
  int i, j, k;
  char b[DFS_BLOCKSIZE];
  char d[DFS_BLOCKSIZE];
  dfs_block block_data;
  int blocknum;
  int lowerbound;
  int fbv_start;
	// STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
	// You need to think of the finer details. You can use bzero() to zero out bytes in memory
  Printf("Test here\n");
  //Initializations and argc check

  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  // sb.valid = 0
  dfs_invalidate();

  // Make sure the disk exists before doing anything else
  disk_create();
  
  //disk_read_block(1, b); 
  
  disksize = 16*1024*1024; // 16MB
  diskblocksize = disk_blocksize();//DISK_BLOCKSIZE;
  //filesystemblocksize = b[7] << 24 | b[6] << 16 | b[5] << 8 | b[4];
  filesystemblocksize = DFS_BLOCKSIZE;//b[7] << 24 | b[6] << 16 | b[5] << 8 | b[4];
  num_filesystem_blocks = disksize / filesystemblocksize;
  

  // Write all inodes as not in use and empty (all zeros)
  //int DiskWriteBlock (uint32 blocknum, disk_block *b) {
  // write disc block
  for(i = 0; i < FDISK_NUM_INODES; i++){ // only FDISK_NUM_INODES inodes
    inodes[i].inuse = 0;
    inodes[i].size = 0;
    inodes[i].bsize = 0;
    for(j = 0; j < filename_size; j++)
      inodes[i].filename[j] = 0;
    for(j = 0; j < 10; j++)
      inodes[i].direct_addr[j] = 0;
    inodes[i].indirect = 0;
    inodes[i].double_indirect = 0;
  }
  i = 0;
  j = 0;
  k = 0;
  blocknum = filesystemblocksize / diskblocksize;
 // Printf("Blocknum: %d ", blocknum);
  while(i < FDISK_NUM_INODES){
    for(j = 0; j < diskblocksize; j++){
//      b[j] = inode[i][k];
      if(k == 0) b[j] = inodes[i].inuse;
      else if(k > 0 && k <= 4)
        b[j] = (inodes[i].size >> (8*(k-1))) & 0xff;
      else if(k > 4 && k <= 8)
        b[j] = (inodes[i].bsize >> (8*(k-5))) & 0xff;
      else if(k > 8 && k <= 47)
        b[j] = inodes[i].filename[(k-9)];
      else if(k > 47 && k <= 87)
        b[j] = (inodes[i].direct_addr[(k-48)/4] >> (8*((k-48)%4))) & 0xff;
      else if(k > 87 && k <= 91)
        b[j] = (inodes[i].indirect >> (8*(k-88))) & 0xff;
      else
        b[j] = (inodes[i].double_indirect >> (8*(k-92))) & 0xff;
      k++;
      if(k == 96){
        i++;
        k = 0;
      }
    }
    disk_write_block(blocknum++, b);
//    i+=diskblocksize;
  }
  //Printf("Blocknum: %d \n", blocknum);
  // Next, setup free block vector (fbv) and write free block vector to the disk
  fbv_start = (blocknum-1) * diskblocksize / filesystemblocksize + 1;
  //Printf("FBV_start: %d\n", fbv_start);
  lowerbound = fbv_start - 1 + DFS_FBV_MAX_NUM_WORDS * 4 / filesystemblocksize;
  Printf("Lowerbound: %d\n", lowerbound);
  for(i = 0; i < DFS_FBV_MAX_NUM_WORDS; i++){
    if(i*32 > lowerbound) fbv[i] = 0xFFFFFFFF;
    else if(lowerbound - i*32 < 32) fbv[i] = 0xFFFFFFFF - (1 << (lowerbound - i*32 + 1)) + 1;
    else fbv[i] = 0;
//    Printf("fbv[%d]: %x\n", i, fbv[i]);
    
  }
  i = 0;
  blocknum = fbv_start;
  //Printf("Start: %d\n", blocknum);
  while(i < DFS_FBV_MAX_NUM_WORDS){
    //Printf("fbv[%d] : %x\n", i, fbv[i]);
    for(j = 0; j < filesystemblocksize; j+=4){
      block_data.data[j] = fbv[i] & 0xFF;
      block_data.data[j+1] = (fbv[i] >> 8) & 0xFF;
      block_data.data[j+2] = (fbv[i] >> 16) & 0xFF;
      block_data.data[j+3] = (fbv[i] >> 24) & 0xFF;
//      Printf("block_data.data[%d] : %x\n", j, block_data.data[j]);
      i++;
      if(i == DFS_FBV_MAX_NUM_WORDS){
        //FdiskWriteBlock(blocknum++, d);
        break;
      }
    }
    //Printf("Write data : %x, to blocknum: %d\n", block_data.data[0], blocknum);
    FdiskWriteBlock(blocknum++, &block_data);
  }
  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  sb.valid = 1;
  sb.blocksize = filesystemblocksize;
  sb.total_block_number = 16*1024*1024 / filesystemblocksize;
  sb.start_inode_block = 1;
  sb.number_of_inodes = 192;
  sb.start_fbv = fbv_start;////lowerbound+1;
  sb.start_block = lowerbound + 1;//blocknum;

  b[0] = sb.valid & 0xFF;
  b[1] = (sb.valid >> 8) & 0xFF;
  b[2] = (sb.valid >> 16) & 0xFF;
  b[3] = (sb.valid >> 24) & 0xFF;
  b[4] = sb.blocksize & 0xFF;
  b[5] = (sb.blocksize >> 8) & 0xFF;
  b[6] = (sb.blocksize >> 16) & 0xFF;
  b[7] = (sb.blocksize >> 24) & 0xFF;
  b[8] = sb.total_block_number & 0xFF;
  b[9] = (sb.total_block_number >> 8) & 0xFF;
  b[10] = (sb.total_block_number >> 16) & 0xFF;
  b[11] = (sb.total_block_number >> 24) & 0xFF;
  b[12] = sb.start_inode_block & 0xFF;
  b[13] = (sb.start_inode_block >> 8) & 0xFF;
  b[14] = (sb.start_inode_block >> 16) & 0xFF;
  b[15] = (sb.start_inode_block >> 24) & 0xFF;
  b[16] = sb.number_of_inodes & 0xFF;
  b[17] = (sb.number_of_inodes >> 8) & 0xFF;
  b[18] = (sb.number_of_inodes >> 16) & 0xFF;
  b[19] = (sb.number_of_inodes >> 24) & 0xFF;
  b[20] = sb.start_fbv & 0xFF;
  b[21] = (sb.start_fbv >> 8) & 0xFF;
  b[22] = (sb.start_fbv >> 16) & 0xFF;
  b[23] = (sb.start_fbv >> 24) & 0xFF;
  b[24] = sb.start_block & 0xFF;
  b[25] = (sb.start_block >> 8) & 0xFF;
  b[26] = (sb.start_block >> 16) & 0xFF;
  b[27] = (sb.start_block >> 24) & 0xFF;
  disk_write_block(FDISK_BOOT_FILESYSTEM_BLOCKNUM*num_filesystem_blocks + 1, b);
  
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {
  // STUDENT: put your code here
  //
  int i = 0;
  int j;
  int times = filesystemblocksize / diskblocksize;
  char db[DFS_BLOCKSIZE/2];

  for(i = 0; i < times; i++){
    for(j = 0; j < diskblocksize; j++){
        db[j] = b->data[diskblocksize*i+j];
    }
    if(disk_write_block(blocknum * times + i, db) == DISK_FAIL) return DISK_FAIL;
    //Printf("Write data to block %d, data: %x\n", blocknum * times + i, db[0]);
  }

  return 1;
}
