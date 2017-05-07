#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

void RunOSTests() {
  // STUDENT: run any os-level tests here
  char* filename = "test_file";
  uint32 handle; 
  int i = 0;
  char mem[10000];
  char Rmem[10000];

  handle = DfsInodeOpen(filename);
  /*
  for(i = 0; i < 520; i++){
    DfsInodeAllocateVirtualBlock(handle, i);
  }
*/
  for(i = 0; i < 10000; i++){
    mem[i] = (char)i;
    printf("%x",mem[i]);
  }
  printf("\n");
  printf("Write %d bytes!!\n", DfsInodeWriteBytes(handle, mem, 10000, 10000));


  printf("Read %d bytes!!\n", DfsInodeReadBytes(handle, Rmem, 10000, 10000));
  printf("\n");
  for(i = 0; i < 10000; i++){
    printf("%x",Rmem[i]);
    
  }
  printf("\n");
  
  
  
  


}

