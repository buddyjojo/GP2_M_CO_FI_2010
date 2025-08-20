
#include <stdio.h>
#include <stdlib.h>

// Convert binary file into HEX bytes for being included in a .c file.

int main(int argc, char* argv[])
{
  FILE* t_fpin;
  int   t_filesize = 0;
  unsigned char* t_buf;
  int  t_i;

  if (2 != argc) {
    printf("Usage: %s <input>n", argv[0]);
    return 0;
  }

  t_fpin = fopen(argv[1], "rb");
  if (NULL == t_fpin)
  {
    printf("[Error]cannot open %s\n", argv[1]);
    return -1;
  }  

  // get file size
  fseek(t_fpin, 0, SEEK_END);
  t_filesize = ftell(t_fpin);
  fseek(t_fpin, 0, SEEK_SET);

  t_buf = (unsigned char*)malloc(t_filesize);
  if (NULL == t_buf)
  {
    printf("[Error]memory allocation.\n");
    return -1;
  }

  fread(t_buf, 1, t_filesize, t_fpin);

  for (t_i = 0; t_i < t_filesize; ++t_i)
  {
    printf("0x%02X,", t_buf[t_i]);
    if (((t_i + 1)%8) == 0) {
       printf("\n");
    }
  }

  fclose(t_fpin);
 
  return 0; 
}

