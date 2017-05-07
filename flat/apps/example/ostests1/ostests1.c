#include "usertraps.h"
#include "misc.h"
#include "ostests.h"
#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

void main (int argc, char *argv[])
{
  char mem[10000];
  char Rmem[10000];
  int i;
  char filename[39] = "Testfile1.txt";
  int fd, fdr;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  if (argc != 3) { 
    Printf("Usage: %s <handle_to_mailbox> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  Printf("Os1: Test Here\n");

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[2], NULL, 10);

  //run_os_tests();
  fd = file_open(filename, "w");

  for(i = 0; i < 10000; i++){
    mem[i] = 'a';
  }
//  for(i = 0; i < 20; i++){
    if(file_write(fd, mem, 3) == -1){
      Printf("Os1: Write failed!!\n");
    }
//  }
  Printf("Os1: Write 3 bytes into the file\n");
  Printf("Os1: Write fid: %d\n", fd);
  file_close(fd);

  //file_delete(filename);

  fdr = file_open(filename, "r");
  Printf("Read fid: %d\n", fdr);
  if(fdr < 0) {
    Printf("Os1: Process1 Fail to open file!\n");
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

  if(file_seek(fdr, 5, FILE_SEEK_CUR) == -1){
    Printf("Os1: Seek error\n");
  }
  else{
    if(file_read(fdr, Rmem, 10) == -1){
      Printf("Os1: Read failed!!\n");
    }
    Printf("Os1: Read 10 bytes from the file\n");

    for(i = 0; i < 10; i++){
      Printf("Os1: %x\n", Rmem[i]);
    }
    Printf("\n");
  }

  file_close(fdr);
  
  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("ostest1 (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("ostest1 (%d): Done!\n", getpid());
}
