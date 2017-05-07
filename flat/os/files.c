#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

extern lock_t fbvlock;
extern lock_t inodelock;
static file_descriptor fd_array[192];
extern dfs_inode inodes[192];
// STUDENT: put your file-level functions here
int FileOpen(char *filename, char *mode){

  int inode_handle;
  file_descriptor* new_fd;
  //printf("Fileopen: %s, %s\n", filename, mode);
  if(mode[0] == 'w'){
    inode_handle = DfsInodeOpen(filename);
  }
  else if(mode[1] == 'w'){
    inode_handle = DfsInodeFilenameExists(filename);
    printf("RW: inode_handle %d\n", inode_handle);
    if(inode_handle == DFS_FAIL){
        inode_handle = DfsInodeOpen(filename);
    }
  }

  else{
    inode_handle = DfsInodeFilenameExists(filename);
    if(inode_handle == DFS_FAIL){
      printf("Fileopen: Cannot find file %s\n", filename);
      //printf("The mode is %s\n", mode);
      return FILE_FAIL;
    }
  }
  LockHandleAcquire(inodelock);
  //printf("Fileopen got lock: %s, %s\n", filename, mode);
  if(fd_array[inode_handle].used != 0){
    printf("The file %s is opened in %s mode!\n", filename, mode);
    LockHandleRelease(inodelock);
    return FILE_FAIL;
  }
  LockHandleRelease(inodelock);
  
  new_fd = &fd_array[inode_handle];


  new_fd->mode = mode; // 0: r / 1: w / 2: rw
  new_fd->used = 1;
  new_fd->filename = filename;
  new_fd->inode_handle = inode_handle;
  new_fd->position = 0;
  new_fd->eof = 0;//inodes[inode_handle].size;
  new_fd->size = inodes[inode_handle].bsize;
  //printf("Successfully open the file %s, size: %d\n", filename, new_fd->size);
  return inode_handle;
}

int FileClose(int handle){
  LockHandleAcquire(inodelock);
  fd_array[handle].used = 0;
  LockHandleRelease(inodelock);
  return FILE_SUCCESS;
}

int FileRead(int handle, void *mem, int num_bytes){
  file_descriptor* fd = &fd_array[handle];
  int count_byte = 0;
  //LockHandleAcquire(inodelock);
  //
  if(handle < 0){
    printf("You did not have a valid handle\n");
    return FILE_FAIL;
  }
  if((fd->used == 0) | (fd->mode[0] == 'w')){
    printf("The file %s is not in the right mode for read %d, %s\n", fd->filename, fd->used, fd->mode);
    return FILE_FAIL;
  }
  count_byte = DfsInodeReadBytes(fd->inode_handle, mem, fd->position, num_bytes);
  fd->position += count_byte;
  if(count_byte < num_bytes) {
    printf("Doesn't read enough bytes in file %s\n", fd->filename);
    return count_byte;
  }

  if(fd->position >= fd->size){
    fd->eof = 1;
  }

  else return count_byte; 

  //LockHandleRelease(inodelock);
  
}

int FileWrite(int handle, void *mem, int num_bytes){
  file_descriptor* fd = &fd_array[handle];
  int count_byte = 0;
  //LockHandleAcquire(inodelock);
  if((fd->used == 0) | (fd->mode[0] == 'r')){
    printf("The file %s is not in the right mode for write %d, %s\n", fd->filename, fd->used, fd->mode);
    //LockHandleRelease(inodelock);
    return FILE_FAIL;
  
  }
 // Allocate bytes?! 
  count_byte = DfsInodeWriteBytes(fd->inode_handle, mem, fd->position, num_bytes);
  fd->position += count_byte;
  fd->size = inodes[fd->inode_handle].bsize;
  if(count_byte < num_bytes) {
    printf("Doesn't write enough bytes in file %s\n", fd->filename);
    return count_byte;
  }
  else {
    fd->eof = 1;
    return count_byte; 
  }

}

int FileSeek(int handle, int num_bytes, int from_where){
  int pos;
  file_descriptor* fd = &fd_array[handle];
  //printf("The file size is: %d\n", fd->size);
  if((fd->used == 0)){
    printf("The file %s is not opened\n", fd->filename);
    return FILE_FAIL;
  }
  if(from_where == FILE_SEEK_CUR){
    pos = fd->position + num_bytes;
  }
  else if(from_where == FILE_SEEK_SET){
    pos = num_bytes;
  }
  else{
    pos = fd->size -1  + num_bytes;
  }
  if(pos >= fd->size) return FILE_FAIL;
  fd->position = pos;
  fd->eof = 0;
  return pos;
}

int FileDelete(char *filename){
  int i = 0;
  int found = 0;
  int filehandle;
  LockHandleAcquire(inodelock);
  filehandle = DfsInodeFilenameExists(filename);
  if(filehandle != DFS_FAIL){
    fd_array[filehandle].used = 0;
    if(DfsInodeDelete(filehandle) == DFS_FAIL){
      printf("Can not delete inode [%d]\n", i);
      LockHandleRelease(inodelock);
      return FILE_FAIL;

    }
  }
  else{
      LockHandleRelease(inodelock);
      printf("Can't find file %s", filename);
      return FILE_FAIL;
  }

  
  LockHandleRelease(inodelock);
  return FILE_SUCCESS;
}
