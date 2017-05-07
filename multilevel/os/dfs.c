#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"
#include "process.h"

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }


// STUDENT: put your file system level functions below.
//

static dfs_superblock sb; // superblock
static uint32 fbv[DFS_FBV_MAX_NUM_WORDS]; // Free block vector

int systemopened = 0;
//int filename_size = 39;
lock_t fbvlock;
lock_t inodelock;
int disk_block_size;

void DfsModuleInit() {
    char mem [FILENAME_SIZE];
// You essentially set the file system as invalid and then open 
// using DfsOpenFileSystem().
  //int i = 0;
  //int diskblock = DiskBytesPerBlock();
  //disk_block b;
  //printf("Here, %x %x %x\n", DiskSize(), diskblock, DiskSize() / diskblock);
  //bzero(b.data, diskblock);
  ////disk_block *b;
  ////DiskReadBlock(1, b);
  ////b->data[0] = 0;
  ////DiskWriteBlock(1, b);
  //  printf("i: %d\n", i);
  //for(i = 0; i < (DiskSize() / diskblock); i++){
  //  DiskWriteBlock(i, &b);
  //}
  sb.valid = 0;
  fbvlock = LockCreate();
  inodelock = LockCreate();
  DfsOpenFileSystem();
 
  if(inodes[0].inuse == 0){
    //printf("Clean inode 0 \n");
    inodes[0].inuse = 1;
    inodes[0].size = 0;
    inodes[0].permission = OR + OW + OX + UR + UW + UX;
    inodes[0].bsize = 0;
    inodes[0].type = DIR;


    mem[0] = '\0';

    DfsInodeWriteBytes(0, mem, 0, FILENAME_SIZE);
  }else{
    //printf("inodes[1] type: %d\n", inodes[1].type);
  }

}

void DfsInvalidate() {
// This is just a one-line function which sets the valid bit of the 
// superblock to 0.
    sb.valid = 0;
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------

int DfsOpenFileSystem() {
  disk_block *b;
  int i, j, k;
  int diskblock;
//Basic steps:
// Check that filesystem is not already open
  if(systemopened != 0) return DFS_FAIL;
// Read superblock from disk.  Note this is using the disk read rather 
// than the DFS read function because the DFS read requires a valid 
// filesystem in memory already, and the filesystem cannot be valid 
// until we read the superblock. Also, we don't know the block size 
// until we read the superblock, either.
  disk_block_size = DiskBytesPerBlock();
//  while(DiskReadBlock(1, b) == DISK_FAIL);
  if(DiskReadBlock(1, b) == DISK_FAIL){
    sb.valid = 0;
    sb.blocksize = 512;//((int)(b->data[7]) << 24) | ((int)(b->data[6]) << 16) | ((int)(b->data[5]) << 8) | ((int)(b->data[4]));
    sb.total_block_number = 16*1024*1024 / sb.blocksize;//((int)(b->data[11]) << 24) | ((int)(b->data[10]) << 16) | ((int)(b->data[9]) << 8) | ((int)(b->data[8]));
    sb.start_inode_block = 1;//((int)(b->data[15]) << 24) | ((int)(b->data[14]) << 16) | ((int)(b->data[13]) << 8) | ((int)(b->data[12]));
    sb.number_of_inodes = 196;//((int)(b->data[19]) << 24) | ((int)(b->data[18]) << 16) | ((int)(b->data[17]) << 8) | ((int)(b->data[16]));
    sb.start_fbv =  37;//((int)(b->data[23]) << 24) | ((int)(b->data[22]) << 16) | ((int)(b->data[21]) << 8) | ((int)(b->data[20]));
    sb.start_block =  39;//((int)(b->data[27]) << 24) | ((int)(b->data[26]) << 16) | ((int)(b->data[25]) << 8) | ((int)(b->data[24]));
  
  }
  else{
//   Copy the data from the block we just read into the superblock in memory
    sb.valid = b->data[0];
//    printf("%x%x%x%x\n", b->data[7], b->data[6], b->data[5], b->data[4]);
//    printf("%x\n", (int)(b->data[5]) << 8);
    sb.blocksize = ((int)(b->data[7]) << 24) | ((int)(b->data[6]) << 16) | ((int)(b->data[5]) << 8) | ((int)(b->data[4]));
    sb.total_block_number = ((int)(b->data[11]) << 24) | ((int)(b->data[10]) << 16) | ((int)(b->data[9]) << 8) | ((int)(b->data[8]));
    sb.start_inode_block = ((int)(b->data[15]) << 24) | ((int)(b->data[14]) << 16) | ((int)(b->data[13]) << 8) | ((int)(b->data[12]));
    sb.number_of_inodes = ((int)(b->data[19]) << 24) | ((int)(b->data[18]) << 16) | ((int)(b->data[17]) << 8) | ((int)(b->data[16]));
    sb.start_fbv =  ((int)(b->data[23]) << 24) | ((int)(b->data[22]) << 16) | ((int)(b->data[21]) << 8) | ((int)(b->data[20]));
    sb.start_block =  ((int)(b->data[27]) << 24) | ((int)(b->data[26]) << 16) | ((int)(b->data[25]) << 8) | ((int)(b->data[24]));
  }
  //printf("b: %x\n", b);
// All other blocks are sized by virtual block size:
// Read inodes
  diskblock = sb.start_inode_block * sb.blocksize / disk_block_size;
  j = 0;
//  printf("Valid: %d\n", sb.valid);
//  printf("Blocksize: %d\n", sb.blocksize);
//  printf("Total_block_number: %d\n", sb.total_block_number);
//  printf("Start_inode_block: %d\n", sb.start_inode_block);
//  printf("Number_of_inodes: %d\n", sb.number_of_inodes);
//  printf("Start_fbv: %d\n", sb.start_fbv);
//  printf("Start_block: %d\n", sb.start_block);
  DiskReadBlock(diskblock++, b);
  for(i = 0; i < sb.number_of_inodes; i++){
    for(k = 0; k < 96; k++){
      if(k == 0) inodes[i].inuse = b->data[j++];
      else if(k > 0 && k <= 4)
        if(k == 1) inodes[i].size = b->data[j++];
        else inodes[i].size |= (int)(b->data[j++]) << 8*(k-1);
      else if(k > 4 && k <= 8)
        if(k == 5) inodes[i].bsize = b->data[j++];
        else inodes[i].bsize |= (int)(b->data[j++]) << 8*(k-5);
      else if(k == 9)
        inodes[i].type = b->data[j++];
      else if(k == 10)
        inodes[i].permission = b->data[j++];
      else if(k == 11)
        inodes[i].ownerid= b->data[j++];
      else if(k > 11 && k <= 47)
        inodes[i].junk[k-12] = b->data[j++];
      else if(k > 47 && k <= 87)
        //b[j] = (inodes[i].direct_addr[(k-48)/4] >> (8*((k-48)%4))) & 0xff;
        if(k%4 == 0) inodes[i].direct_addr[(k-48)/4] = (int)(b->data[j++]);
        else inodes[i].direct_addr[(k-48)/4] |= (int)(b->data[j++]) << (8*((k-48)%4));
      else if(k > 87 && k <= 91)
        //b[j] = (inodes[i].indirect >> (8*(k-88))) & 0xff;
        if(k == 88) inodes[i].indirect = (int)(b->data[j++]);
        else inodes[i].indirect |= (int)(b->data[j++]) << (8*((k-88)));
      else
        //b[j] = (inodes[i].double_indirect >> (8*(k-92))) & 0xff;
        if(k == 92) inodes[i].double_indirect = (int)(b->data[j++]);
        else inodes[i].double_indirect |= (int)(b->data[j++]) <<(8*(k-92));
      if(k == 95 && i == sb.number_of_inodes-1) break;
      if(j == disk_block_size){
        j = 0;
        DiskReadBlock(diskblock++, b);
      }

    }
    //printf("%d: inuse: %d name: %s\n ", i, inodes[i].inuse, inodes[i].filename);
  
  }
// Read free block vector
  diskblock = sb.start_fbv * sb.blocksize / disk_block_size;
  i = 0; 
  while(i < DFS_FBV_MAX_NUM_WORDS){
    DiskReadBlock(diskblock++, b);
    
    for(j = 0; j < disk_block_size; j+= 4){
      fbv[i] = ((int)(b->data[j+3]) << 24) | ((int)(b->data[j+2]) << 16) | ((int)(b->data[j+1]) << 8) | ((int)(b->data[j]));
      //printf("fbv[%d]: %x\n", i, fbv[i]);
      i++;
      if(i == DFS_FBV_MAX_NUM_WORDS){
        //FdiskWriteBlock(blocknum++, d);
        break;
      }
    }
  }
  
// Change superblock to be invalid, write back to disk, then change 
// it back to be valid in memory
  sb.valid = 0;
  DfsWriteSuperBlock();
  sb.valid = 1;
  
  systemopened = 1;
  return DFS_SUCCESS;
}

int DfsWriteSuperBlock(){
  disk_block b;
  b.data[0] = sb.valid & 0xFF;
  b.data[1] = (sb.valid >> 8) & 0xFF;
  b.data[2] = (sb.valid >> 16) & 0xFF;
  b.data[3] = (sb.valid >> 24) & 0xFF;
  b.data[4] = sb.blocksize & 0xFF;
  b.data[5] = (sb.blocksize >> 8) & 0xFF;
  b.data[6] = (sb.blocksize >> 16) & 0xFF;
  b.data[7] = (sb.blocksize >> 24) & 0xFF;
  b.data[8] = sb.total_block_number & 0xFF;
  b.data[9] = (sb.total_block_number >> 8) & 0xFF;
  b.data[10] = (sb.total_block_number >> 16) & 0xFF;
  b.data[11] = (sb.total_block_number >> 24) & 0xFF;
  b.data[12] = sb.start_inode_block & 0xFF;
  b.data[13] = (sb.start_inode_block >> 8) & 0xFF;
  b.data[14] = (sb.start_inode_block >> 16) & 0xFF;
  b.data[15] = (sb.start_inode_block >> 24) & 0xFF;
  b.data[16] = sb.number_of_inodes & 0xFF;
  b.data[17] = (sb.number_of_inodes >> 8) & 0xFF;
  b.data[18] = (sb.number_of_inodes >> 16) & 0xFF;
  b.data[19] = (sb.number_of_inodes >> 24) & 0xFF;
  b.data[20] = sb.start_fbv & 0xFF;
  b.data[21] = (sb.start_fbv >> 8) & 0xFF;
  b.data[22] = (sb.start_fbv >> 16) & 0xFF;
  b.data[23] = (sb.start_fbv >> 24) & 0xFF;
  b.data[24] = sb.start_block & 0xFF;
  b.data[25] = (sb.start_block >> 8) & 0xFF;
  b.data[26] = (sb.start_block >> 16) & 0xFF;
  b.data[27] = (sb.start_block >> 24) & 0xFF;
  return DiskWriteBlock (1, &b); 


}

//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {
  disk_block b;
  int i, j, k;
  int blocknum;
  if(sb.valid == 0) return DFS_FAIL;
  //----------------------
  //  Write SB back
  //----------------------
  b.data[0] = sb.valid & 0xFF;
  b.data[1] = (sb.valid >> 8) & 0xFF;
  b.data[2] = (sb.valid >> 16) & 0xFF;
  b.data[3] = (sb.valid >> 24) & 0xFF;
  b.data[4] = sb.blocksize & 0xFF;
  b.data[5] = (sb.blocksize >> 8) & 0xFF;
  b.data[6] = (sb.blocksize >> 16) & 0xFF;
  b.data[7] = (sb.blocksize >> 24) & 0xFF;
  b.data[8] = sb.total_block_number & 0xFF;
  b.data[9] = (sb.total_block_number >> 8) & 0xFF;
  b.data[10] = (sb.total_block_number >> 16) & 0xFF;
  b.data[11] = (sb.total_block_number >> 24) & 0xFF;
  b.data[12] = sb.start_inode_block & 0xFF;
  b.data[13] = (sb.start_inode_block >> 8) & 0xFF;
  b.data[14] = (sb.start_inode_block >> 16) & 0xFF;
  b.data[15] = (sb.start_inode_block >> 24) & 0xFF;
  b.data[16] = sb.number_of_inodes & 0xFF;
  b.data[17] = (sb.number_of_inodes >> 8) & 0xFF;
  b.data[18] = (sb.number_of_inodes >> 16) & 0xFF;
  b.data[19] = (sb.number_of_inodes >> 24) & 0xFF;
  b.data[20] = sb.start_fbv & 0xFF;
  b.data[21] = (sb.start_fbv >> 8) & 0xFF;
  b.data[22] = (sb.start_fbv >> 16) & 0xFF;
  b.data[23] = (sb.start_fbv >> 24) & 0xFF;
  b.data[24] = sb.start_block & 0xFF;
  b.data[25] = (sb.start_block >> 8) & 0xFF;
  b.data[26] = (sb.start_block >> 16) & 0xFF;
  b.data[27] = (sb.start_block >> 24) & 0xFF;
  DiskWriteBlock(1, &b);

  //----------------------
  //  Write inodes back
  //----------------------
  i = 0;
  j = 0;
  k = 0;
  blocknum = sb.start_inode_block * sb.blocksize/ disk_block_size;
  printf("inodes[1] type: %d\n", inodes[1].type);
  while(i < DFS_INODE_MAX_NUM){
    for(j = 0; j < disk_block_size; j++){
//      b.data[j] = inode[i][k];
      if(k == 0) b.data[j] = inodes[i].inuse;
      else if(k > 0 && k <= 4)
        b.data[j] = (inodes[i].size >> (8*(k-1))) & 0xff;
      else if(k > 4 && k <= 8)
        b.data[j] = (inodes[i].bsize >> (8*(k-5))) & 0xff;
      else if(k == 9)
        b.data[j] = inodes[i].type;
      else if(k == 10)
        b.data[j] = inodes[i].permission;
      else if(k == 11)
        b.data[j] = inodes[i].ownerid;
      else if(k > 11 && k <= 47)
        b.data[j] = inodes[i].junk[(k-12)];
      else if(k > 47 && k <= 87)
        b.data[j] = (inodes[i].direct_addr[(k-48)/4] >> (8*((k-48)%4))) & 0xff;
      else if(k > 87 && k <= 91)
        b.data[j] = (inodes[i].indirect >> (8*(k-88))) & 0xff;
      else
        b.data[j] = (inodes[i].double_indirect >> (8*(k-92))) & 0xff;
      k++;
      if(k == 96){
        i++;
        k = 0;
      }
    }
    DiskWriteBlock(blocknum++, &b);
  }
  //----------------------
  //  Write FBV back
  //----------------------
  i = 0;
  blocknum = sb.start_fbv * sb.blocksize/ disk_block_size;
  while(i < DFS_FBV_MAX_NUM_WORDS){
    for(j = 0; j < disk_block_size; j+=4){
      b.data[j] = fbv[i] & 0xFF;
      b.data[j+1] = (fbv[i] >> 8) & 0xFF;
      b.data[j+2] = (fbv[i] >> 16) & 0xFF;
      b.data[j+3] = (fbv[i] >> 24) & 0xFF;
      i++;
      if(i == DFS_FBV_MAX_NUM_WORDS){
        //FdiskWriteBlock(blocknum++, d);
        break;
      }
    }
    DiskWriteBlock(blocknum++, &b);
  }
  sb.valid = 0;
  return DFS_SUCCESS;

}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

uint32 DfsAllocateBlock() {
  int addr32, addr;
// Check that file system has been validly loaded into memory
  if(sb.valid == 0) return DFS_FAIL;
// Find the first free block using the free block vector (FBV), mark it in use
// Return handle to block
  LockHandleAcquire(fbvlock);
  for(addr32 = 0; addr32 < DFS_FBV_MAX_NUM_WORDS; addr32++){
//    printf("Find fbv addr32: %x\n", fbv[addr32]);
    if(fbv[addr32] != 0){
//      printf("In fbv addr32: %x\n", fbv[addr32]);
      for(addr = 0; addr < 32; addr++){
//        printf("In fbv addr: %d: %x\n", addr, fbv[addr32] & ((uint32)( 1 ) << addr));
        if((fbv[addr32] & ((uint32)( 1 ) << addr)) != 0){
          //printf("fbv addr32: %x, %d, %d\n", fbv[addr32], addr32, addr);
          if(addr < 31)
            fbv[addr32] &= ~((uint32)( 1 ) << addr);
          else
            fbv[addr32] = 0;
          //printf("fbv addr32: %x\n", fbv[addr32]);
          LockHandleRelease(fbvlock);
          return addr32 * 32 + addr;
          
        }
      }
    }

  }
  LockHandleRelease(fbvlock);
  return DFS_FAIL; // Cannot find one empty space

}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

//int DfsFreeBlock(uint32 blocknum) {
//  if(sb.valid == 0) return DFS_FAIL;
//  LockHandleAcquire(fbvlock);
//  fbv[blocknum/32] |= (1 << blocknum%32);
//  LockHandleRelease(fbvlock);
//  return DFS_SUCCESS;
//}

int DfsFreeBlock(uint32 blockid){
  uint32 wordid = blockid /32;
  if(sb.valid == 0) return DFS_FAIL;
  LockHandleAcquire(fbvlock);
  
  if(blockid >= DFS_FBV_MAX_NUM_WORDS) {

    LockHandleRelease(fbvlock);
    return DFS_FAIL;
  }
  //if(blockid == 0) {
  //  
  //  LockHandleRelease(fbvlock);
  //  return DFS_FAIL; // Nothing change because it means the inode does not use this block
  //}
  blockid %= 32;
  fbv[wordid] |= 1 << blockid;
  LockHandleRelease(fbvlock);
  return DFS_SUCCESS;

}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {
  disk_block temp_d;
  int i;
  int totalnumber = 0;
  int bytecount;
  for(i = 0; i < sb.blocksize; i += disk_block_size){
    bytecount = DiskReadBlock(blocknum*sb.blocksize/disk_block_size + i/disk_block_size, &temp_d);
    if(bytecount == DISK_FAIL){
      break;
    }
    totalnumber += bytecount;
    CopyCharArray((b->data) + i, temp_d.data, bytecount);
    
  }
  if(totalnumber == 0) return DFS_FAIL;
  else return totalnumber;
  

}

void CopyCharArray(char * dest, char * source, int size){
  int i = 0;

  for(i = 0; i < size; i++){
    dest[i] = source[i];
  }
}

//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b){
  disk_block temp_d;
  int i;
  int totalnumber = 0;
  int bytecount;
//  printf("Write byte %d\n", blocknum);
  for(i = 0; i < sb.blocksize; i += disk_block_size){
    CopyCharArray(temp_d.data, (b->data) + i, bytecount);
    bytecount = DiskWriteBlock(blocknum*sb.blocksize/disk_block_size + i/disk_block_size, &temp_d);
    if(bytecount == DISK_FAIL){
      break;
    }
    totalnumber += bytecount;
    
  }
  if(totalnumber == 0) return DFS_FAIL;
  else return totalnumber;
  
  
}


////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------

int DfsInodeFilenameExists(uint32 handle, char *filename) {
  int i = 0;
  char mem[76];
  if(inodes[handle].inuse == FALSE){
    printf("DfsInodeFilenameExists: This is not a valid inode\n");
    return INVALID_INODE;
  }

  if(inodes[handle].type != DIR){
    printf("DfsInodeFilenameExists: This is not a directory\n");
    return NOT_A_DIRECTORY;
  }

  if(checkPermission(handle, X) != OK) {
    printf("DfsInodeFilenameExists: Permission Deny!!\n");
    return PERMISSION_DENIED;
  }

//  printf("inodes [%d], size: %d\n", handle, inodes[handle].bsize);
  for(i = 0 ; i < inodes[handle].bsize; i+=FILENAME_SIZE){
    DfsInodeReadBytes(handle, mem, i, FILENAME_SIZE-4);
    //printf("DfsInodeFilenameExists: %d: inuse: %d match %d name: %s, compare_name: %s\n ", i, inodes[i].inuse, matchfilename(mem, filename), mem, filename);
    if(matchfilename(mem, filename) == 1 ){
      //printf("DfsInodeFilenameExists: %d: inuse: %d match %d name: %s, compare_name: %s\n ", i, inodes[i].inuse, matchfilename(mem, filename), mem, filename);
      DfsInodeReadBytes(handle, mem, i+FILENAME_SIZE-4, 4);
      return  ((int)(mem[3]) << 24) | ((int)(mem[2]) << 16) | ((int)(mem[1]) << 8) | ((int)(mem[0]));
    }
  }
  //printf("DfsInodeFilenameExists: File does not exist!!\n");
  return DOES_NOT_EXIST;
}


int CheckFolderEmpty(int handle){
  char mem [76];
  DfsInodeReadBytes(handle, mem , 0, FILENAME_SIZE);
  if(mem[0] != '0')
    return 0;
  else
    return 1;


}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

int DfsInodeOpen(uint32 handle, char *filename) {
  uint32 inode_index = DfsInodeFilenameExists(handle, filename);
  int i = 0;
  int c = 0;
  char mem [FILENAME_SIZE];
  //printf("DfsInodeOpen: handle: %d inode_index %d\n", handle, inode_index);
  if(inode_index == DOES_NOT_EXIST){
    for(i = 0; i < sb.number_of_inodes; i++){
      if(inodes[i].inuse == 0){
        break;
      }
    }
    if(i == sb.number_of_inodes) return DFS_FAIL;


    LockHandleAcquire(inodelock);


    inodes[i].inuse = 1;
    inodes[i].size = 0;
    inodes[i].permission = UR + UW + UX;
    inodes[i].bsize = 0;
    inodes[i].type = FILE;
    inodes[i].ownerid = GetCurrentPid();

//    printf("DfsInodeOpen: Creating ");
    for(c = 0; c < FILENAME_SIZE; c++){
      if(c < FILENAME_SIZE - 4){
        mem[c] = filename[c];
      }
      else{
        mem[c] = (i >> (4*(c + 4 - FILENAME_SIZE))) & 0xFF;
      }
      //printf("%x", mem[c]);
    }
//    printf("\n");
//    printf("DfsInodeOpen: found the file: %d\n", i);
    DfsInodeWriteBytes(handle, mem, inodes[handle].bsize-FILENAME_SIZE, FILENAME_SIZE);
    mem[0] = '\0';
    DfsInodeWriteBytes(handle, mem, inodes[handle].bsize, FILENAME_SIZE);

    LockHandleRelease(inodelock);
    //printf("DfsInodeOpen: son: %d\n", i);
    //printf("DfsInodeOpen: type: %d\n", inodes[i].type);
    //printf("DfsInodeOpen: address: %x\n", inodes);
    return i;

  }
  else //if(inode_index >= 0)
    return inode_index;
    //DfsInodeDelete(inode_index);
    //return DfsInodeOpen(handle, filename);
}


//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {
  int i = 0;
  int j = 0;
  int blockid;
  dfs_block blockdata;
  int doubleblockid;
  dfs_block doubleblockdata;
  uint32 block_count = 0;
  uint32 indirect_addr;
  uint32 block_amount;

  block_amount = inodes[handle].size / sb.blocksize + (inodes[handle].size % sb.blocksize != 0);
  if(inodes[handle].inuse == 0){
    return DFS_SUCCESS;
  }
  LockHandleAcquire(inodelock);
  if(checkPermission(handle, W) != OK) {
    printf("DfsInodeDelete: The inode[%d] permission = %x\n", handle, inodes[handle].permission);   
    printf("DfsInodeDelete: Permission Deny\n");
    LockHandleRelease(inodelock);
    return PERMISSION_DENIED;
  }
 
  for(i = 0; i < 10; i++){
    if(block_count >= block_amount){
      printf("Clean inode\n");
      inodes[handle].size = 0;
      inodes[handle].bsize = 0;
      inodes[handle].inuse = 0;
      LockHandleRelease(inodelock);
      return DFS_SUCCESS;
    }
//    printf("The data: %d\n", inodes[handle].direct_addr[i]);
    if(DfsFreeBlock(inodes[handle].direct_addr[i]) == DFS_FAIL){
      printf("Out of range of inode %d, Direct addr %d, which is %d\n", handle, i, inodes[handle].direct_addr[i]);
      LockHandleRelease(inodelock);
      return DFS_FAIL;
    }
    inodes[handle].direct_addr[i] = 0;
    block_count+=1;//sb.blocksize; 
    if(block_count >= block_amount){
//      printf("Clean inode\n");
      inodes[handle].size = 0;
      inodes[handle].bsize = 0;
      inodes[handle].inuse = 0;
      LockHandleRelease(inodelock);
      return DFS_SUCCESS;
    }
  }
  
  blockid = inodes[handle].indirect;
  if(blockid == 0){
    LockHandleRelease(inodelock);
    printf("Data missing of inode %d\n", handle);
    return DFS_FAIL;
  }
  else{ // single-indirect exists

    if(DfsReadBlock(blockid, &blockdata) == DFS_FAIL) {
      printf("Cannot read blockid: %d\n", blockid);
      return DFS_FAIL;
    }

    for(i = 0; i < sb.blocksize; i+=4){
      indirect_addr = (uint32)blockdata.data[i+3] << 24 | (uint32)blockdata.data[i+2] << 16 | (uint32)blockdata.data[i+1] << 8 | (uint32)blockdata.data[i];
      if(DfsFreeBlock(indirect_addr) == DFS_FAIL){
        LockHandleRelease(inodelock);
        printf("Out of range of inode %d, Indirect addr %d\n", handle, indirect_addr);
        return DFS_FAIL;
      
      }
      block_count+=sb.blocksize; 
      if(block_count >= block_amount){
        inodes[handle].indirect = 0;
        inodes[handle].size = 0;
        inodes[handle].bsize = 0;
        inodes[handle].inuse = 0;
        LockHandleRelease(inodelock);
        return DFS_SUCCESS;
      }

    }
    inodes[handle].indirect = 0;
  }

  
  doubleblockid = inodes[handle].double_indirect;
  if(doubleblockid == 0){
    printf("Data missing of inode's double_indirect %d\n", handle);
    LockHandleRelease(inodelock);
    return DFS_FAIL;
  
  }
  

  if(DfsReadBlock(doubleblockid, &doubleblockdata) == DFS_FAIL){
    LockHandleRelease(inodelock);
    return DFS_FAIL;
  }

  for(j = 0; j < sb.blocksize; j+=4){
    blockid = (uint32)doubleblockdata.data[j+3] << 24 | (uint32)doubleblockdata.data[j+2] << 16 | (uint32)doubleblockdata.data[j+1] << 8 | (uint32)doubleblockdata.data[j];

    if(blockid == 0){
      LockHandleRelease(inodelock);
      printf("Data missing of inode %d\n", handle);
    return DFS_FAIL;
    }


    if(DfsReadBlock(blockid, &blockdata) == DFS_FAIL) {
      LockHandleRelease(inodelock);
      return DFS_FAIL;
    }

    for(i = 0; i < sb.blocksize; i+=4){
      indirect_addr = (uint32)blockdata.data[i+3] << 24 | (uint32)blockdata.data[i+2] << 16 | (uint32)blockdata.data[i+1] << 8 | (uint32)blockdata.data[i];
      if(DfsFreeBlock(indirect_addr) == DFS_FAIL){
        printf("Out of range of inode %d, Indirect addr %d\n", handle, indirect_addr);
        LockHandleRelease(inodelock);
        return DFS_FAIL;
      
      }
      block_count+=sb.blocksize; 
      if(block_count >= block_amount){
        inodes[handle].double_indirect = 0;
        inodes[handle].size = 0;
        inodes[handle].bsize = 0;
        inodes[handle].inuse = 0;
        LockHandleRelease(inodelock);
        return DFS_SUCCESS;
      }

    }


  }
  //inodes[handle].double_indirect = 0;


  inodes[handle].size = 0;
  inodes[handle].bsize = 0;
  inodes[handle].inuse = 0;
  LockHandleRelease(inodelock);
  return DFS_FAIL;
}



//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  int read_byte = 0;
  int virtual_block_num;
  int first_byte;
  int i = 0;
  int real_block_num;
  dfs_block b;
  LockHandleAcquire(inodelock);
  if(checkPermission(handle, R) != OK){
    printf("DfsInodeReadBytes: Permission Deny\n");
    LockHandleRelease(inodelock);
    return PERMISSION_DENIED;
  }
  LockHandleRelease(inodelock);

  virtual_block_num = start_byte / sb.blocksize;
  first_byte = start_byte % sb.blocksize;


  while((read_byte < num_bytes) && (start_byte + read_byte < inodes[handle].bsize)){
    real_block_num  = DfsInodeTranslateVirtualToFilesys(handle, virtual_block_num++);  
    if(real_block_num == DFS_FAIL) return DFS_FAIL;
    if(DfsReadBlock(real_block_num, &b) == DFS_FAIL) return DFS_FAIL;
    for(i = first_byte; i < sb.blocksize; i++){
      *(char*)mem = b.data[i];
      mem++;
      read_byte++;
      
      if((read_byte >= num_bytes) || (start_byte + read_byte >= inodes[handle].bsize)) break;//return read_bytes;
    }
  
    first_byte = 0;
  }
  return read_byte;
}


//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------

int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  int write_byte = 0;
  int virtual_block_num;
  int first_byte;
  int i = 0;
  int real_block_num;
  
  dfs_block b;

//  printf("Write DfsInodeWriteBytes byte: %d\n", start_byte);
  LockHandleAcquire(inodelock);
  if(checkPermission(handle, W) != OK){
    printf("DfsInodeWriteBytes: Permission Deny\n");
    LockHandleRelease(inodelock);
    return PERMISSION_DENIED;
  }

  virtual_block_num = start_byte / sb.blocksize;
  first_byte = start_byte % sb.blocksize;  
  while(write_byte < num_bytes){
    while((start_byte + write_byte) >= inodes[handle].size ) {
      if(DfsInodeAllocateVirtualBlock(handle, (start_byte + write_byte)/ sb.blocksize) == DFS_FAIL){
        printf("DfsInodeWriteBytes: Cannot allocate a new disk block for write.\n");
        return DFS_FAIL;
      }
    }
    real_block_num  = DfsInodeTranslateVirtualToFilesys(handle, virtual_block_num++); 
    if(real_block_num == DFS_FAIL) {
      printf("DfsInodeWriteBytes: DfsInodeTranslateVirtualToFilesys Error\n");
      return DFS_FAIL;
    }
    if(DfsReadBlock(real_block_num, &b) == DFS_FAIL) {
      printf("DfsInodeWriteBytes: DfsReadBlock read block_num %d Error\n", real_block_num);
      return DFS_FAIL;
    }
    for(i = first_byte; i < sb.blocksize; i++){
      b.data[i] = *(char*)mem;
      mem++;
      write_byte++;
      if(write_byte >= num_bytes) break;//return read_bytes;
    }
    DfsWriteBlock(real_block_num, &b);   
    first_byte = 0;
    if(start_byte + write_byte > inodes[handle].bsize){
      inodes[handle].bsize = start_byte + write_byte ;
    }
        
  
  }
  return write_byte;
}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeFilesize(uint32 handle) {
  if(handle > DFS_INODE_MAX_NUM) {
    printf("Not a valid inode\n");
    return DFS_FAIL;
  }
  return inodes[handle].bsize;

}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------

int DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {
//  int nowsize = inodes[handle].size / sb.blocksize;
  int nowsize = inodes[handle].size / sb.blocksize + (inodes[handle].size % sb.blocksize != 0);
  uint32 free_addr;
  uint32 free_layer;
  uint32 first_layer_addr; 
  dfs_block blockdata;
  uint32 secondlayer_addr;
  free_addr = DfsAllocateBlock();
  if(free_addr == DFS_FAIL){
    printf("DfsInodeAllocateVirtualBlock: Cannot allocate block\n");
    return DFS_FAIL;
  }
  
  LockHandleAcquire(inodelock);
  if(virtual_blocknum < 10){
    inodes[handle].direct_addr[virtual_blocknum] = free_addr;
//    printf("Allocate direct addr : %d to addr %d\n", virtual_blocknum, free_addr);
  }
  else if(virtual_blocknum < sb.blocksize/4 + 10){
    virtual_blocknum -= 10;
//    printf("Allocate indirect addr : %d\n", virtual_blocknum);
    if(nowsize == 10){
      free_layer = DfsAllocateBlock();
      inodes[handle].indirect = free_layer;
//      printf("Allocate indirect addr to %d\n", inodes[handle].indirect);
    }
    if(DfsReadBlock(inodes[handle].indirect, &blockdata) == DFS_FAIL){
      LockHandleRelease(inodelock);
      printf("DfsInodeAllocateVirtualBlock reads illegal indirect address %d.\n", inodes[handle].indirect);
      return DFS_FAIL;
    }
    blockdata.data[virtual_blocknum*4    ] = free_addr & 0xff;
    blockdata.data[virtual_blocknum*4 + 1] = (free_addr >> 8) & 0xff;
    blockdata.data[virtual_blocknum*4 + 2] = (free_addr >> 16) & 0xff;
    blockdata.data[virtual_blocknum*4 + 3] = (free_addr >> 24) & 0xff;
    DfsWriteBlock(inodes[handle].indirect, &blockdata);
    
  }
  else{
//    printf("Allocate double layer: %d\n", nowsize);
    if(nowsize == 10 + sb.blocksize/4){
      free_layer = DfsAllocateBlock();
      inodes[handle].double_indirect = free_layer;
    }
    virtual_blocknum -= (sb.blocksize/4 + 10);
    nowsize -= (sb.blocksize/4 + 10);
    if(virtual_blocknum >= (sb.blocksize/4) * (sb.blocksize/4)) {
      LockHandleRelease(inodelock);
      printf("DfsInodeAllocateVirtualBlock wants to allocate too large block.\n");
      return DFS_FAIL;
    }
    first_layer_addr =  virtual_blocknum / (sb.blocksize/4);
     if(DfsReadBlock(inodes[handle].double_indirect, &blockdata) == DFS_FAIL) {
      LockHandleRelease(inodelock);
      printf("DfsInodeAllocateVirtualBlock reads illegal double indirect address %d.\n", inodes[handle].double_indirect);
      return DFS_FAIL;
    }   
    if(nowsize % (sb.blocksize/4) == 0){
      free_layer = DfsAllocateBlock();
      blockdata.data[first_layer_addr + 0] =  free_layer & 0xff;
      blockdata.data[first_layer_addr + 1] = (free_layer >> 8) & 0xff;
      blockdata.data[first_layer_addr + 2] = (free_layer >> 16) & 0xff;
      blockdata.data[first_layer_addr + 3] = (free_layer >> 24) & 0xff;
      DfsWriteBlock(inodes[handle].double_indirect, &blockdata) ;
    }

    secondlayer_addr = ((uint32)blockdata.data[first_layer_addr+3] << 24) | ((uint32)blockdata.data[first_layer_addr+2] << 16) | ((uint32)blockdata.data[first_layer_addr+1] << 8) | ((uint32)blockdata.data[first_layer_addr]);
    
    if(DfsReadBlock(secondlayer_addr, &blockdata) == DFS_FAIL) {
      LockHandleRelease(inodelock);
      printf("DfsInodeAllocateVirtualBlock reads illegal second layer of double indirect address %d.\n", secondlayer_addr);
      return DFS_FAIL;
    }
    virtual_blocknum %= (sb.blocksize/4);
    blockdata.data[virtual_blocknum*4    ] =  free_addr & 0xff;
    blockdata.data[virtual_blocknum*4 + 1] = (free_addr >> 8) & 0xff;
    blockdata.data[virtual_blocknum*4 + 2] = (free_addr >> 16) & 0xff;
    blockdata.data[virtual_blocknum*4 + 3] = (free_addr >> 24) & 0xff;
    DfsWriteBlock(secondlayer_addr, &blockdata) ;





  
  }
  

  inodes[handle].size += sb.blocksize;
  LockHandleRelease(inodelock);
  return free_addr;
}



//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

int DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {
  uint32 addr;
  uint32 first_layer_addr; 
  dfs_block blockdata;
  uint32 secondlayer_addr;

  LockHandleAcquire(inodelock);

  if(virtual_blocknum < 10){
    LockHandleRelease(inodelock);
//    printf("DfsInodeAllocateVirtualBlock: return addr %d from direct_addr: %d\n", inodes[handle].direct_addr[virtual_blocknum], virtual_blocknum);
    return inodes[handle].direct_addr[virtual_blocknum];
  }
  else if(virtual_blocknum < sb.blocksize/4 + 10){
    virtual_blocknum -= 10;
    if(DfsReadBlock(inodes[handle].indirect, &blockdata) == DFS_FAIL){
      printf("DfsInodeTranslateVirtualToFilesys reads illegal indirect address %d.\n", inodes[handle].indirect);
      return DFS_FAIL;
    }
//    printf("DfsInodeTranslateVirtualToFilesys reads indirect address %d.\n", inodes[handle].indirect);

    addr =  ((uint32) blockdata.data[virtual_blocknum*4 + 3] << 24) | ((uint32) blockdata.data[virtual_blocknum*4 + 2] << 16) | ((uint32) blockdata.data[virtual_blocknum*4 + 1] << 8) | ((uint32) blockdata.data[virtual_blocknum*4]);
    LockHandleRelease(inodelock);
//    printf("DfsInodeAllocateVirtualBlock: return addr: %d\n", addr);
    return addr;
    
  }
  else{
    virtual_blocknum -= (sb.blocksize/4 + 10);
    if(virtual_blocknum >= (sb.blocksize/4) * (sb.blocksize/4)) {
      printf("DfsInodeAllocateVirtualBlock wants to allocate too large block.\n");
      return DFS_FAIL;
    }
    first_layer_addr =  virtual_blocknum / (sb.blocksize/4);
    if(DfsReadBlock(inodes[handle].double_indirect, &blockdata) == DFS_FAIL) {
      printf("DfsInodeAllocateVirtualBlock reads illegal double indirect address %d.\n", inodes[handle].double_indirect);
      return DFS_FAIL;
    }
    secondlayer_addr = ((uint32)blockdata.data[first_layer_addr+3] << 24) | ((uint32)blockdata.data[first_layer_addr+2] << 16) | ((uint32)blockdata.data[first_layer_addr+1] << 8) | ((uint32)blockdata.data[first_layer_addr]);
    
    if(DfsReadBlock(secondlayer_addr, &blockdata) == DFS_FAIL) {
      printf("DfsInodeAllocateVirtualBlock reads illegal second layer of double indirect address %d.\n", secondlayer_addr);
      return DFS_FAIL;
    }
    virtual_blocknum %= (sb.blocksize/4);
    addr =  ((uint32) blockdata.data[virtual_blocknum*4 + 3] << 24) | ((uint32) blockdata.data[virtual_blocknum*4 + 2] << 16) | ((uint32) blockdata.data[virtual_blocknum*4 + 1] << 8) | ((uint32) blockdata.data[virtual_blocknum*4]);
    LockHandleRelease(inodelock);
//    printf("DfsInodeAllocateVirtualBlock: return addr: %d\n", addr);
    return addr; 
  }


}

uint32 matchfilename(char* filename, char * comparename){
  int i = 0;

  for(i = 0; i < FILENAME_SIZE; i++){
    if((comparename[i] == '\0')  && (comparename[i] == '\0')) return 1;
    if(filename[i] != comparename[i]){
      //printf("Different in %d %x %x\n", i, filename[i], comparename[i]);
      return 0;
    }
  }
  return 1;
}

int checkPermission(uint32 id, uint32 mode)
{
  dfs_inode *node;
  uint32 flmode;

  node = &inodes[id];

  if(node==NULL)
  {
    return 0;				//inode not valid
  }
  
  flmode = node->permission;

//  printf("CheckPermission: id: %d, Current: %d\n", node->ownerid, GetCurrentPid());
  //printf("CheckPermission: flmode %x, mode %x\n", flmode, mode);
  if(node->ownerid==GetCurrentPid())
  {
    mode<<=3;
  }
  
  flmode &= mode;			//Mask all the bits other than the
  					//bits being tested
  flmode ^= mode;			//XOR with bits being tested. Result
  					//is 0 only if all the bits match 

  if(flmode)
  {
    return 0;				//permission denied
  }
  else
  {
    return OK;				//permission granted
  }
}  

