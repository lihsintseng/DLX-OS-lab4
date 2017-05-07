#include "usertraps.h"
#include "ostests.h"

void main (int argc, char *argv[])
{
  char mem[10000];
  char Rmem[10000];
  int i;
  //char "Testfile1.txt"[39] = "Testfile1.txt";
  int fd, fdr;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done


  if(mkdir("Test1", UR+UW+UX) == -1) {
    //Printf("Make Directory Fail\n");
  }

  // Convert the command-line strings into integers for use as handles
 // s_procs_completed = dstrtol(argv[2], NULL, 10);

  //run_os_tests();
  fd = file_open("Test1/Testfile1.txt", "w");

  for(i = 0; i < 10000; i++){
    mem[i] = 'b';
  }
  if(file_write(fd, mem, 4095) == -1){
    Printf("Os1: Write failed!!\n");
  }
  Printf("Os1: Write 4095 bytes into the file\n");
  Printf("Os1: Write fid: %d\n", fd);
  file_close(fd);

  fdr = file_open("Test1/Testfile1.txt", "r");
  Printf("Os1: Read fid: %d\n", fdr);
  if(fdr < 0) {
    Printf("Os1: Process 1 fail to open file!\n");
    return;
  }
  file_seek(fdr, 10, FILE_SEEK_SET);
  if(file_read(fdr, Rmem, 10) == -1){
    Printf("Os1: Read failed!!\n");
  }
  Printf("Os1: Read 10 bytes from the file\n");

  for(i = 0; i < 10; i++){
    Printf("Os1: %x\n", Rmem[i]);
  }
  Printf("\n");

  file_seek(fdr, 5, FILE_SEEK_CUR);
  if(file_read(fdr, Rmem, 10) == -1){
    Printf("Os1: Read failed!!\n");
  }
  Printf("Os1: Read 10 bytes from the file\n");

  for(i = 0; i < 10; i++){
    Printf("Os1: %x\n", Rmem[i]);
  }
  Printf("\n");


  file_close(fdr);
  
  // Signal the semaphore to tell the original process that we're done
  //if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
  //  Printf("ostest1 (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
  //  Exit();
  //}

  Printf("ostest1 (%d): Done!\n", getpid());
}
