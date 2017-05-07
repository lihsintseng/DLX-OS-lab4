#ifndef __FILES_SHARED__
#define __FILES_SHARED__

#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

#define FILE_MAX_FILENAME_LENGTH 76

#define FILE_MAX_READWRITE_BYTES 4096


typedef struct file_descriptor {
  // STUDENT: put file descriptor info here
  char* mode; 
  int used;
  char* filename;
  int inode_handle;
  int position;
  int eof;
  int size;
} file_descriptor;

int FileOpen(char *, char *);
int FileClose(int );
int FileRead(int , void *, int );
int FileWrite(int , void *, int ) ;
int FileSeek(int , int , int );
int FileDelete(char *);

#define FILE_FAIL -1
#define FILE_EOF -1
#define FILE_SUCCESS 1

#endif
