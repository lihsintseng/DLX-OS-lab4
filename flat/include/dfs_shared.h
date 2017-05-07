#ifndef __DFS_SHARED__
#define __DFS_SHARED__

#define DFS_BLOCKSIZE 512  // Must be an integer multiple of the disk blocksize
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
#define DFS_INODE_MAX_NUM 192
#define DFS_FBV_MAX_NUM_WORDS (16*1024*1024 / DFS_BLOCKSIZE / 32)
typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;
#define filename_size (96 - 1 - 4 - 4 - 4*10 - 4 - 4)
typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 96 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 96 bytes.
  char inuse;
  uint32 size;
  uint32 bsize;
  char filename[filename_size];//[96-];
  uint32 direct_addr[10];
  uint32 indirect;
  uint32 double_indirect;
} dfs_inode;
void CopyCharArray(char * , char * , int);
uint32 matchfilename(char* , char *);

#define DFS_MAX_FILESYSTEM_SIZE 0x1000000  // 16MB

#define DFS_FAIL -1
#define DFS_SUCCESS 1



#endif
