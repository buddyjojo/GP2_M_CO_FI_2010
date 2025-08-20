#ifndef CHECKSUM_H
#define CHECKSUM_H

//extern unsigned int adler;

void InitCheckSum(unsigned int *p_adler);
void UpdateCheckSum(unsigned char* ptr, int len, unsigned int *p_adler);

#if 0
void crco_add_frame(uchar* y, uchar* u, uchar* v,
                    int width, int height,
                    int stride_y, int stride_uv, 
                    int active_x, int active_y
                    );
#endif

#endif
