#include "usertraps.h"

#define OR 4
#define OW 2
#define OX 1
#define UR 32
#define UW 16
#define UX 8
#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

void main (int argc, char *argv[])
{
//  run_os_tests();
//
  char mem[10000];
  char Rmem[10000];
  int i;
  int fd;
  int fdr;
  int readbyte;

  if(mkdir("Test1", UR+UW+UX) == -1) {
    Printf("Make Directory Fail\n");
  }
  
//  file_delete("/Test1/test.txt");
  fd = file_open("/Test1/test.txt", "W");

  for(i = 0; i < 10000; i++){
   // mem[i] = (char)i;
    mem[i] = 'a';
//    Printf("%x",mem[i]);
  }
  if(file_write(fd, mem, 1) == -1){
    Printf("Write failed!!\n");
  }
  Printf("Write 4095 bytes into the file\n");
  Printf("Write fid: %d\n", fd);
  file_close(fd);

////  file_delete(filename);


  //
//  fd = file_open("/Test1/test2.txt", "W");
//
//  for(i = 0; i < 10000; i++){
//    mem[i] = (char)234;
////    Printf("%x",mem[i]);
//  }
//  if(file_write(fd, mem, 4095) == -1){
//    Printf("Write failed!!\n");
//  }
//  Printf("Write 4095 bytes into the file\n");
//  Printf("Write fid: %d\n", fd);
//  file_close(fd);



  fdr = file_open("Test1/test.txt", "r");

  Printf("Read fid: %d\n", fdr);
  if(fdr < 0) {
    Printf("Fail to open file!\n");
    return;
  }
 // file_seek(fdr, 10, FILE_SEEK_SET);
  readbyte = file_read(fdr, Rmem, 10);
  if(readbyte == -1){
    Printf("Read failed!!\n");
  }
  Printf("Read %d bytes from the file\n", readbyte);

  for(i = 0; i < 10; i++){
    Printf("%x", Rmem[i]);
  }
  Printf("\n");

  if(file_seek(fdr, 5, FILE_SEEK_CUR) < 0){
    Printf("Can't seek the file!\n");
  }
  readbyte = file_read(fdr, Rmem, 10);
  if(readbyte == -1){
    Printf("Read failed!!\n");
  }
  Printf("Read %d bytes from the file\n", readbyte);

  for(i = 0; i < 10; i++){
    Printf("%x", Rmem[i]);
  }
  Printf("\n");


  file_close(fdr);


  //fdr = file_open("Test1/test2.txt", "r");

  //Printf("Read fid: %d\n", fdr);
  //if(fdr < 0) {
  //  Printf("Fail to open file!\n");
  //  return;
  //}
  //file_seek(fdr, 10, FILE_SEEK_SET);
  //if(file_read(fdr, Rmem, 10) == -1){
  //  Printf("Read failed!!\n");
  //}
  //Printf("Read 10 bytes from the file\n");

  //for(i = 0; i < 10; i++){
  //  Printf("%x", Rmem[i]);
  //}
  //Printf("\n");

  //file_seek(fdr, 5, FILE_SEEK_CUR);
  //if(file_read(fdr, Rmem, 10) == -1){
  //  Printf("Read failed!!\n");
  //}
  //Printf("Read 10 bytes from the file\n");

  //for(i = 0; i < 10; i++){
  //  Printf("%x", Rmem[i]);
  //}
  //Printf("\n");


  //file_close(fdr);



}
