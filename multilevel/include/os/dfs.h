#ifndef __DFS_H__
#define __DFS_H__

#include "dfs_shared.h"

dfs_inode inodes[192]; // all inodes
int DfsWriteSuperBlock();
uint32 matchfilename(char* , char * );

#endif
