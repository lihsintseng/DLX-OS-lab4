#ifndef __DFS_SHARED__
#define __DFS_SHARED__

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  uint32 valid;
  uint32 blocksize;
  uint32 total_block_number;
  uint32 start_inode_block;
  uint32 number_of_inodes;
  uint32 start_fbv;
  uint32 start_block;



} dfs_superblock;

#define DFS_BLOCKSIZE 512  // Must be an integer multiple of the disk blocksize
#define DFS_INODE_MAX_NUM 192
#define DFS_FBV_MAX_NUM_WORDS (16*1024*1024 / DFS_BLOCKSIZE / 32)
typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;
typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 96 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 96 bytes.
  char inuse;
  char type; // directory or regular file
  int permission; // file permission
  char ownerid;
  uint32 size;
  uint32 bsize;
  uint32 direct_addr[10];
  uint32 indirect;
  uint32 double_indirect;
  char junk[96 -1*4 -4 -4 -4*10 -4 -4];
} dfs_inode;

int checkPermission(uint32, uint32 );
int CheckFolderEmpty(int);
int DfsInodeDelete(uint32);
int DfsInodeReadBytes(uint32, void *, int, int);
int DfsInodeOpen(uint32, char *);
int DfsInodeFilenameExists(uint32, char *);
int DfsInodeWriteBytes(uint32, void *, int, int);

#define DFS_MAX_FILESYSTEM_SIZE 0x1000000  // 16MB

#define DFS_FAIL -1
#define DFS_SUCCESS 1

#define FILENAME_SIZE 76

#define TRUE	1
#define FALSE   0
#define OR 4
#define OW 2
#define OX 1
#define UR 32
#define UW 16
#define UX 8
#define OK 1
#define FILE 1
#define DIR 2
#define R 4
#define W 2
#define X 1

//Inode for the root directory
#define ROOT_DIR 	0x0

#define INVALID_FILE		-1
#define INVALID_INODE		-10
#define FILE_EXISTS		-2
#define DOES_NOT_EXIST		-3
#define DIRECTORY_EXISTS	-4
#define NOT_A_DIRECTORY		-5
#define NOT_A_FILE		-6
#define INVALID_PATH		-7
#define PERMISSION_DENIED	-8
#define DIRECTORY_NOT_EMPTY	-9

#endif
