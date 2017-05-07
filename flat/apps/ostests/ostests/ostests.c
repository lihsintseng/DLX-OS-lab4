#include "usertraps.h"
#include "ostests.h"
#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

void main (int argc, char *argv[])
{
  char mem[10000];
  char Rmem[10000];
  int i;
  char filename[39] = "Testfile.txt";
  int fd, fdr;

  Printf("Run OS TESTS !!!!\n");

  run_os_tests();
  Printf("End OS TESTS !!!!\n");
  Printf("\n");
  Printf("\n");
  Printf("\n");
  Printf("\n");
  fd = file_open(filename, "rw");

  for(i = 0; i < 10000; i++){
    mem[i] = (char)i;
//    Printf("%x",mem[i]);
  }
  for(i = 0; i < 20; i++){
    if(file_write(fd, mem, 4095) == -1){
      Printf("Write failed!!\n");
    }
  }
  Printf("Write 4095 bytes into the file\n");
  Printf("Write fid: %d\n", fd);
  file_close(fd);

  //file_delete(filename);

  fdr = file_open(filename, "r");
  Printf("Read fid: %d\n", fdr);
  if(fdr < 0) {
    Printf("Fail to open file!\n");
    return;
  }
  file_seek(fdr, 10, FILE_SEEK_SET);
  if(file_read(fdr, Rmem, 10) == -1){
    Printf("Read failed!!\n");
  }
  Printf("Read 10 bytes from the file\n");

  for(i = 0; i < 10; i++){
    Printf("%x", Rmem[i]);
  }
  Printf("\n");

  file_seek(fdr, 5, FILE_SEEK_CUR);
  if(file_read(fdr, Rmem, 10) == -1){
    Printf("Read failed!!\n");
  }
  Printf("Read 10 bytes from the file\n");

  for(i = 0; i < 10; i++){
    Printf("%x", Rmem[i]);
  }
  Printf("\n");


  file_close(fdr);

}
