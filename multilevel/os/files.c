#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"
#include "dfs_shared.h"

// STUDENT: put your file-level functions here
static char *str="";
static file_descriptor fd_array[192];

extern lock_t fbvlock;
extern lock_t inodelock;
extern dfs_inode inodes[192]; // all inodes

extern PCB	*currentPCB;

void PrintCase(int value, char* s){
  switch(value){
    case(INVALID_FILE):
      printf("%s: This is an Invalid file\n", s);
      break;
    case(INVALID_INODE):	
      printf("%s: This is an Invalid inode\n", s);
      break;
    case(FILE_EXISTS):	
      printf("%s: The file exists\n", s);
      break;
    case(DOES_NOT_EXIST):
      printf("%s: The file does not exist\n", s);
      break;
    case(DIRECTORY_EXISTS):
      printf("%s: The directory exists\n", s);
      break;
    case(NOT_A_DIRECTORY):		
      printf("%s: This is not a directory\n", s);
      break;
    case(NOT_A_FILE):
      printf("%s: This is not a file\n", s);
      break;
    case(INVALID_PATH):
      printf("%s: This is an Invalid path\n", s);
      break;
    case(PERMISSION_DENIED):
      printf("%s: Permission Denied!\n", s);
      break;
    case(DIRECTORY_NOT_EMPTY):
      printf("%s: The directory is not empty\n", s);
      break; 
  
  
  }




}

int FO(int handle, char *filename, uint32 mode){
  file_descriptor* new_fd;
  int inode_handle;
  if(mode == FILE_W){
    inode_handle = DfsInodeOpen(handle, filename);
//    printf("Successfully open the inode, %d, type: %d\n", inode_handle, inodes[inode_handle].type);
//    printf("FO: address: %d\n", handle);
    if((*str == '\0') && (inodes[inode_handle].bsize != 0)){ // Clean existed write file
//      printf("FO: Clean existed write file.\n");
      DfsInodeDelete(inode_handle);
      inode_handle = DfsInodeOpen(handle, filename);
    }

    if(inode_handle < 0){
      PrintCase(inode_handle, "FO");
      return FILE_FAIL;
    }
  }
  else if(mode == FILE_RW){
    inode_handle = DfsInodeFilenameExists(handle, filename);
    if(inode_handle < 0){
        inode_handle = DfsInodeOpen(handle, filename);
    } 
  
  }
  else{
    inode_handle = DfsInodeFilenameExists(handle, filename);
    if(inode_handle < 0){
      PrintCase(inode_handle, "FO");
      printf("Fileopen: Cannot find file %s\n", filename);
      printf("The mode is %d\n", mode);
      return FILE_FAIL;
    }
  }
  LockHandleAcquire(inodelock);
  if((fd_array[inode_handle].used != 0) && (inodes[inode_handle].type != DIR) ){
    printf("The file %s is opened!\n", filename);
    LockHandleRelease(inodelock);
    return FILE_FAIL;
  }
  LockHandleRelease(inodelock);
  if(inodes[inode_handle].type == FILE) {
    new_fd = &fd_array[inode_handle];


    new_fd->mode = mode; // 0: r / 1: w / 2: rw
    new_fd->used = 1;
//    new_fd->filename = filename;
    dstrncpy(new_fd->filename, filename, FILE_MAX_FILENAME_LENGTH-4); 
    new_fd->inode_handle = inode_handle;
    new_fd->position = 0;
    new_fd->id = GetCurrentPid();
    new_fd->size = inodes[inode_handle].bsize;
//    printf("Successfully open the file %s, %d, type: %d size: %d\n", fd_array[inode_handle].filename, inode_handle, inodes[inode_handle].type, new_fd->size);
  }
  return inode_handle;
  
}

int MakeInodeFromPath(char *path, int type, int openornot, char* dstname){
  int inode_handle = 0;
  char dst [4096];
  int count = 1;
  int handle = 0;
  int i;
  int parent;
  char mem[76];
  file_descriptor* new_fd;
  count = getOneName(dst, path);
  if(dst[0] != '/'){
    handle = currentPCB->currentDIR; 
  }
  parent = handle; 
  //dstname = NULL;
  mem[0] = '\0';
  while(count != 0){
    inode_handle = DfsInodeFilenameExists(handle, dst);
//    printf("MakeInodeFromPath: handle: %d, dst: %s, %s, and the new inode %d\n", handle, dst, str, inode_handle);
    if(inode_handle == DOES_NOT_EXIST){
      if(*str == '\0'){ // The leaf file
        if(checkPermission(handle, W) != OK){
          printf("MakeInodeFromPath: Permission Deny!\n");
          return PERMISSION_DENIED;
        }
        if(openornot != 0){
//          printf("MakeInodeFromPath: Open file for %s\n", dst);
          inode_handle = DfsInodeOpen(handle, dst);   
//          printf("MakeInodeFromPath: New Handle: %d\n", inode_handle);
          inodes[inode_handle].type = type;
//          printf("MakeInodeFromPath: The permission: %d\n", inodes[inode_handle].permission);
          if(type == DIR){
            DfsInodeWriteBytes(inode_handle, mem, 0, FILENAME_SIZE);
          }
          return inode_handle;
        }
        else{
          for(i = 0; i < FILENAME_SIZE -4; i++){
            dstname[i] = dst[i];
          }
          //printf("MakeInodeFromPath: return its parent, its name is %s\n", dstname);
          return handle;
        }
      }
      else{
        printf("MakeInodeFromPath: Path does not exist\n");
        return INVALID_PATH;
      }
    }
    else if(inode_handle > 0){
      if(*str == '\0'){ // The leaf file
        printf("MakeInodeFromPath: File exists\n");
        return FILE_EXISTS;
      }
      if(checkPermission(handle, X) != OK){
        printf("MakeInodeFromPath: Permission Deny\n");
        return PERMISSION_DENIED;
      }
      parent = handle; 
      handle = inode_handle;
      count = getOneName(dst, path);
    }
    else{
      printf("MakeInodeFromPath: Something wrong!!\n");
      return FILE_FAIL;
      
    }
  }
  return inode_handle;


}

int TraInodeFromPath(char *path, int *parent){
  int inode_handle = 0;
  char dst [4096];
  int count = 1;
  file_descriptor* new_fd;
  int handle = 0;
  count = getOneName(dst, path);
  //printf("TraInodeFromPath: get: %s %s\n", dst, str);
  if(dst[0] != '/'){
    handle = currentPCB->currentDIR; 
  }
  *parent = handle;
  while(count != 0){
    //printf("TraInodeFromPath: handle: %d %s\n", handle, dst);
    inode_handle = DfsInodeFilenameExists(handle, dst);
    //printf("TraInodeFromPath: inode_handle: %d %s\n", inode_handle, dst);
    if(inode_handle < 0){
      return INVALID_PATH;
      //if(*str == '\0'){
      //  if(checkPermission(handle, W) != OK){
      //    printf("TraInodeFromPath: Permission Deny!\n");
      //    return PERMISSION_DENIED;
      //  }
      //  printf("TraInodeFromPath: Open file for it\n");
      //  inode_handle = DfsInodeOpen(handle);   
      //  inodes[inode_handle].type = type;
      //  return inode_handle;
      //}
      //else{
      //  printf("TraInodeFromPath: Path does not exist\n");
      //  return INVALID_PATH;
      //}
    }
    else{
      if(*str == '\0'){
        *parent = handle;
        printf("TraInodeFromPath: File exists\n");
        return inode_handle;
      }
      if(checkPermission(handle, X) != OK){
        printf("TraInodeFromPath: Permission Deny\n");
        return PERMISSION_DENIED;
      }
      *parent = handle;
      handle = inode_handle;
      count = getOneName(dst, NULL);
      //printf("TraInodeFromPath: get: %s %s\n", dst, str);
    }
  }
  printf("TraInodeFromPath: handle %d, parent %d inode_handle %d", handle, *parent, inode_handle);
  return inode_handle;


}


int FileOpen(char *filename, char *mode){
  int inode_handle = 0;
  file_descriptor* new_fd;
  char dst [4096];
  int count = 1;
  int i = 0;
  int intmode;
  count = getOneName(dst, filename);
  //printf("FileOpen: Count: %d, dst: %s\n", count, dst);
  if(dst[0] != '/'){
    inode_handle = currentPCB->currentDIR; 
  }

  intmode = convertModeToInt(mode);
  while(count != 0){
    if(count < 0){
      printf("FileOpen: Get name failed\n");
      return FILE_FAIL;
    }
    inode_handle = FO(inode_handle, dst, intmode);
//    printf("inode_handle: %d, type: %d, filename: %s\n", inode_handle, inodes[inode_handle].type, dst);
    

//    printf("FileOpen: open file %s of inode %d\n", fd_array[inode_handle].filename, inode_handle);
    count = getOneName(dst, NULL);
    //printf("FileOpen: Count: %d, dst: %s\n", count, dst);
    if(count != 0) {
      if(inodes[inode_handle].type != DIR){
        printf("FileOpen: Cannot open a file as a directory\n");
        return FILE_FAIL;
      }
    }
  } // end wile
  if(inodes[inode_handle].type != FILE){
    if(intmode != FILE_R && intmode != FILE_RW){
      printf("FileOpen: You should write a file, not a directory, the mode is %d\n", intmode);
      return FILE_FAIL;
    }

    LockHandleAcquire(inodelock);
    new_fd = &fd_array[inode_handle];


    new_fd->mode = intmode; // 0: r / 1: w / 2: rw
    new_fd->used = 1;
    dstrncpy(new_fd->filename, filename, FILE_MAX_FILENAME_LENGTH-4); 
    new_fd->inode_handle = inode_handle;
    new_fd->position = 0;
    new_fd->id = GetCurrentPid();
    new_fd->size = inodes[inode_handle].bsize;

    LockHandleRelease(inodelock);
  }
  if(checkPermission(inode_handle, intmode) != OK){
    printf("FileOpen: Permission Denied\n");
    return FILE_FAIL;
  }
 

  return inode_handle;



//  printf("Fileopen: %s, %x\n", filename, filename[13]);
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
 // printf("FileRead: read file %s\n", fd_array[handle].filename);
  if(handle < 0){
    printf("You did not have a valid handle\n");
    return FILE_FAIL;
  }
  if((fd->used == 0) | (fd->mode == FILE_W)  | (fd->mode == FILE_RW)){
    printf("The file %s is not in the right mode for read %d, %d the handle is %d\n", fd->filename, fd->used, fd->mode, handle);
    return FILE_FAIL;
  }
  count_byte = DfsInodeReadBytes(fd->inode_handle, mem, fd->position, num_bytes);
  fd->position += count_byte;
  if(count_byte < num_bytes) {
    printf("Doesn't read enough bytes in file %s\n", fd->filename);
    return count_byte;
  }

  return count_byte; 

  //LockHandleRelease(inodelock);
  
}

int FileWrite(int handle, void *mem, int num_bytes){
  file_descriptor* fd = &fd_array[handle];
  int count_byte = 0;
  //LockHandleAcquire(inodelock);
//  printf("FileWrite: write file %s in handle %d, with position %d\n", fd_array[handle].filename, handle, fd->position);
  if((fd->used == 0) | (fd->mode == FILE_R)){
    printf("The file %s is not in the right mode for write %d, %d\n", fd->filename, fd->used, fd->mode);
    //LockHandleRelease(inodelock);
    return FILE_FAIL;
  
  }
 // Allocate bytes?! 
  count_byte = DfsInodeWriteBytes(fd->inode_handle, mem, fd->position, num_bytes);
  fd->position += count_byte;
  fd->size = inodes[fd->inode_handle].bsize;
  if(count_byte < num_bytes) {
    printf("Doesn't write enough bytes in file %s\n", fd->filename);
    return FILE_FAIL;
  }
  else {
//    fd->eof = 1;
    return count_byte; 
  }

}

// Link a file to a new name in the file system. srcpath is the name of an existing file and dstpath is the new 
// (additional) name the file will have after the link call is completed. A process should be able to access a 
// file through any of the path name created explicitly or through the link call. If the srcpath is a directory, 
// then return -1. On all errors, return -1. On success, return 0.
// Example usage: result = link("/a/foo","/newfoo");
 

int FileLink(char *srcpath, char *dstpath){
  int dstinode;
  int dstparent;
  int srcinode;
  int srcparent;
  int intmode;
  int count;
  char dst [FILENAME_SIZE - 4];
  char mem [FILENAME_SIZE];
  int i , c;
  //printf("FileLink: inside\n");
  srcinode = TraInodeFromPath(srcpath, &srcparent);
  //printf("\tFileLink: get srcinode %d\n", srcinode);
  if(srcinode < 0){
    PrintCase(srcinode, "FileLink");
    return FILE_FAIL;
  }
  dstparent = MakeInodeFromPath(dstpath, FILE, 0, dst); 
  //printf("\tFileLink: get dstparent %d\n", dstparent);
  //printf("\tFileLink: get dst %s\n", dst);
  
  if(dstparent< 0){
    PrintCase(dstparent, "FileLink");
    return FILE_FAIL;
  }

  if(checkPermission(dstparent, W) != OK){
    printf("FileLink: Permission Denied\n");
    return FILE_FAIL;
  }

  
  LockHandleAcquire(inodelock);
  for(c = 0; c < FILENAME_SIZE; c++){
    if(c < FILENAME_SIZE - 4){
      mem[c] = dst[c];
    }
    else{
      mem[c] = (srcinode >> (4*(c + 4 - FILENAME_SIZE))) & 0xFF;
    }
  }
  //printf("\tFileLink: get the dst filename %s\n", dst);
//int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  DfsInodeWriteBytes(dstparent, mem, inodes[dstparent].bsize-FILENAME_SIZE, FILENAME_SIZE);
  mem[0] = '\0';
  DfsInodeWriteBytes(dstparent, mem, inodes[dstparent].bsize, FILENAME_SIZE);

  LockHandleRelease(inodelock);
  
  return 0;

}

// Create the specified directory. All directories leading to the leaf must be executable. The parent directory
// must additionally be writable and the leaf should not already exist. Return FILE_FAIL on failure, and 
// FILE_SUCCESS on success.

int MkDir(char *path, int permissions){
  int inode;
  char dstname [72];
  inode =  MakeInodeFromPath(path, DIR, 1, dstname);

  if(inode < 0){
    PrintCase(inode, "MkDir");
    return FILE_FAIL;
  }
  else{
    inodes[inode].permission = permissions;
    return FILE_SUCCESS;
  }


}

// This function behaves identical to FileDelete() except on a directory. Additionally, the leaf must be empty. 
// (eg, all directory entries must be '\0'.) If called on the root directory, this function should fail. If the 
// leaf is not empty, this function fails. Return FILE_FAIL on failure, and FILE_SUCCESS on success.
int RmDir(char *path){
  int inode;
  int parentnode;
  char dstname[72];
  int i, c;
  char mem [76];
  int readid;
  int found = 0;

  
  inode = TraInodeFromPath(path, &parentnode);
  LockHandleAcquire(inodelock);
  if(inode < 0){
    PrintCase(inode, "RmDir");
    LockHandleRelease(inodelock);
    return FILE_FAIL;
  }
  
  if(inodes[inode].type != DIR){
    printf("RmDir: This is not a directory\n");
    LockHandleRelease(inodelock);
    return FILE_FAIL;
  }

  if(checkPermission(parentnode, W) != OK){
    printf("RmDir: Permission Denied\n");
    LockHandleRelease(inodelock);
    return FILE_FAIL;
  }

  if(DfsInodeReadBytes(inode, mem, 0, FILENAME_SIZE) == DFS_FAIL){
    printf("RmDir: Cannot Read the directory\n");
    LockHandleRelease(inodelock);
    return FILE_FAIL;
  }

  if(mem[0] != '\0'){
    printf("RmDir: Directory is not empty\n");
    LockHandleRelease(inodelock);
    return FILE_FAIL;
  }

  if(DfsInodeDelete(inode) == DFS_FAIL){
    printf("Can not delete inode [%d]\n", i);
    LockHandleRelease(inodelock);
    return FILE_FAIL;

  }

  for(i = 0; i < inodes[parentnode].bsize; i += FILENAME_SIZE){
    if(DfsInodeReadBytes(parentnode, mem, i, FILENAME_SIZE) == DFS_FAIL){
      LockHandleRelease(inodelock);
      return FILE_FAIL;
    } 
    if(found == 1){
      DfsInodeWriteBytes(parentnode, mem, i-FILENAME_SIZE, FILENAME_SIZE);
    }
    //if(mem[0] == '\0'){
    //  printf("Something Wrong\n"); 
    //  LockHandleRelease(inodelock);
    //  return FILE_FAIL;
    //}
    readid = (int) mem[FILENAME_SIZE-1] << 24 | (int) mem[FILENAME_SIZE-2] << 16 | (int) mem[FILENAME_SIZE-3] << 8 | (int) mem[FILENAME_SIZE-4];
    if((readid == inode) && (mem[0] != '0')){
      found = 1;  
    }
  }
  inodes[parentnode].bsize -= FILENAME_SIZE;
 // printf("RmDir: inodes[%d] size: %d\n", parentnode, inodes[parentnode].bsize);

  LockHandleRelease(inodelock);
  if(currentPCB->currentDIR == readid);
    currentPCB->currentDIR = parentnode;
  return FILE_SUCCESS;
}


// Delete the file specified by path. All directories leading to the leaf 
// must be executable. The leaf node must be a file (not a directory). The 
// parent directory must additionally be writable. The process invoking 
// this function must also own the leaf (permissions of the leaf do not 
// matter - only ownership). On success, this function should clear the 
// directory entry corresponding to the leaf and free all associated 
// resources. Return FILE_FAIL on failure, and FILE_SUCCESS on success.

int FileDelete(char *path){
  int inode;
  int inodeparent;
  int i, c;
  char mem [76];
  int readid;
  int found = 0;
//int TraInodeFromPath(char *path, int& parent){

  inode = TraInodeFromPath(path, &inodeparent);
  //printf("FileDelete: inode %d, inodeparent %d\n", inode, inodeparent);
  if(inode < 0){
    PrintCase(inode, "FileDelete");
    return FILE_FAIL;
  }
  
  LockHandleAcquire(inodelock);
  if(fd_array[inode].used == 1){
    printf("FileDelete: File is in used\n");
    LockHandleRelease(inodelock);
    return FILE_FAIL;
  }
  fd_array[inode].used == 0;
  fd_array[inode].position = 0;
  fd_array[inode].size = 0;
  if(inodes[inode].type != FILE){
    printf("FileDelete: This is not a file\n");
    LockHandleRelease(inodelock);
    return FILE_FAIL;
  }
  if(DfsInodeDelete(inode) == DFS_FAIL){
    printf("Can not delete inode [%d]\n", i);
    LockHandleRelease(inodelock);
    return FILE_FAIL;

  }
  if(checkPermission(inodeparent, W) != OK){
    printf("FileDelete Permission Denied\n");
    LockHandleRelease(inodelock);
    return FILE_FAIL;
  }
  //printf("FileDelete inodeparent %d\n", inodeparent);
  for(i = 0; i < inodes[inodeparent].bsize; i += FILENAME_SIZE){
    if(DfsInodeReadBytes(inodeparent, mem, i, FILENAME_SIZE) == DFS_FAIL){
      LockHandleRelease(inodelock);
      return FILE_FAIL;
    } 
    if(found == 1){
      DfsInodeWriteBytes(inodeparent, mem, i-FILENAME_SIZE, FILENAME_SIZE);
    }
    //if(mem[0] == '\0'){
    //  printf("Something Wrong\n"); 
    //  LockHandleRelease(inodelock);
    //  return FILE_FAIL;
    //}
    readid = (int) mem[FILENAME_SIZE-1] << 24 | (int) mem[FILENAME_SIZE-2] << 16 | (int) mem[FILENAME_SIZE-3] << 8 | (int) mem[FILENAME_SIZE-4];
    if((readid == inode) && (mem[0] != '\0')){
      found = 1;  
    }
  }
  inodes[inodeparent].bsize -= FILENAME_SIZE;

  LockHandleRelease(inodelock);
//  printf("FileDelete: now parent size: %d", inodes[inodeparent].bsize);
  return FILE_SUCCESS;

  
}

//--------------------------------------------------------------------------
//	convertModeToInt
//	
//	Converts a mode string to integer. Mode "r" is 1, mode "w" is 2. The various 
//  combinations are obtained by adding these basic modes.
//--------------------------------------------------------------------------
int convertModeToInt(char *mode)
{
  int i;
  int md = 0;

  for(i=0; mode[i]!='\0'; i++)
  {
    switch(mode[i])
    {
      case 'r':
      case 'R':
        md |= FILE_R;
	break;
      case 'w':
      case 'W':
        md |= FILE_W;
	break;
      default:
        return md |= FILE_RW;
    }
  }
  return md;
}

//---------------------------------------------------------------------------
//	getOneName
//	
//	This function works much like strtok. The only difference is that the
//	field delimiter is fixed at '/', and you have to specify the
//	destination string where the token could be returned. The string dst
//	should have at least 31 bytes of space. This function also checks for
//	some error conditions such as extremely long filenames and successive
//	'/' characters. On success it returns the length of the string written
//	in dst (not to exceed 30). On error, it returns -1. If the string src
//	starts with a '/', the first '/' is ignored.
//
//	Example: Consider src = "/a/b/cd"
//		len = getOneName(dst, src); 
//			//returns dst = "a", len = 1;
//		len = getOneName(dst, NULL);
//			//returns dst = "b", len = 1;
//		len = getOneName(dst, NULL);
//			//returns dst = "cd", len = 2;
//		len = getOneName(dst, NULL);
//			//returns dst = "", len = 0;
//	
//	Note that the successive calls should be made with src = NULL. If src
//	is not NULL, the string passed is parsed from the beginnning.
//--------------------------------------------------------------------------

int getOneName(char *dst, char *src)
{
  int count = 0;

  if(src!=NULL)
  {
    if(*src=='/')
      src++;
    if(*src=='/')
      return -1;			//successive '/' not allowed
    str = src;	
  }
  for(;*str!='\0';str++)
  {
    dst[count] = *str;
    count++;
    if(*str=='/')
    {
      str++;
      count--;
      if(*str =='/')
      {
        return -1;			//successive '/' not allowed
      }
      break;
    }
    else
    {
      if(count==71) 
      {
       return -1;			//Filename too long
      }
    }
  }
  dst[count] = '\0';
  return count;
}

int FileSeek(int handle, int num_bytes, int from_where){
  int pos;
  file_descriptor* fd = &fd_array[handle];

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

//  printf("The file size is: %d, the position is %d\n", fd->size, pos);
  if(pos >= fd->size) return FILE_FAIL;

  fd->position = pos;
 // fd->eof = 0;
  return pos;
}


