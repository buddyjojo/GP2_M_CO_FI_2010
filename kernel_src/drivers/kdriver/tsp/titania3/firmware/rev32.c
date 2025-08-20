
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    FILE* t_fpin;
    FILE* t_fpout;
    unsigned char* t_buf;
    unsigned char* t_ptr;
    unsigned long t_size;
    unsigned long t_count;

    if (argc != 3)
    {
        printf("%s <input> <output>\n", argv[0]);
        exit(1);
    }

    t_fpin = fopen(argv[1], "rb");
    if (NULL == t_fpin)
    {
        printf("[Error]cannot open %s\n", argv[1]);
        exit(1);
    }

    t_fpout = fopen(argv[2], "wb");
    if (NULL == t_fpout)
    {
        fclose(t_fpin);
        printf("[Error]cannot open %s\n", argv[2]);
        exit(1);
    }

    
    fseek(t_fpin, 0, SEEK_END);
    t_size = ftell(t_fpin);
    fseek(t_fpin, 0, SEEK_SET);

    t_buf = (unsigned char*)malloc(t_size);
    fread(t_buf, t_size, 1, t_fpin);

    if (t_size % 4)
    {
        printf("[Error] file size is not multiple of 4 bytes.\n");
        free(t_buf);
        fclose(t_fpin);
        fclose(t_fpout);
    }

    t_count = t_size >> 2;
    t_ptr = t_buf;
    while (t_count > 0)
    {
        t_ptr[0] ^= t_ptr[3];
        t_ptr[3] ^= t_ptr[0];
        t_ptr[0] ^= t_ptr[3];
        t_ptr[1] ^= t_ptr[2];
        t_ptr[2] ^= t_ptr[1];
        t_ptr[1] ^= t_ptr[2];
        t_ptr += 4;
        --t_count;
    }

    fwrite(t_buf, t_size, 1, t_fpout);
    free(t_buf);

    fclose(t_fpin);
    fclose(t_fpout);
}
