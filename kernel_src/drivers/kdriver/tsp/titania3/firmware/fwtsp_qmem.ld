

MEMORY
{
    coderam : ORIGIN = 0x00000000, LENGTH = 0x00002800
    dataram : ORIGIN = 0x00002800, LENGTH = 0x00000800
    dbgram  : ORIGIN = 0x00004000, LENGTH = 0x00000400
    shmram  : ORIGIN = 0x00004800, LENGTH = 0x00000400
}


SECTIONS
{
/*
    00000000    +-------------------------------+
                | Reset                         |
    00000x00    +-------------------------------+
                | Code                          |
                |                               |
    00002000    +-------------------------------+
                | Data                          |
    00003000    +-------------------------------+
                |                               |
    00004000    +-------------------------------+
                | Debug                         |
    00004400    +-------------------------------+
                |                               |
    00004800    +-------------------------------+
                | Shared                        |
    00004C00    +-------------------------------+
                |                               |
    00005000    +-------------------------------+
*/


    .text :
    {
        *(.text)
    } > coderam


    .rodata :
    {
        _data_start = .;

        *(.rodata)
    } > dataram

    .rodata.str1.1 :
    {
        *(.rodata.str1.1)
    } > dataram

    .data :
    {
        *(.data)

        _data_end = .;
    } > dataram

    .bss :
    {
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    } > dataram

    _src_data_start = LOADADDR(.rodata);
    _src_data_end = LOADADDR(.bss);

    .stack :
    {
        *(.stack)
    } > dataram

    .debug :
    {
        _dbg_fw = .;
        LONG(0x0);
        _dbg_flag = .;
        LONG(0x0);
        LONG(0x0);
        LONG(0x0);

        LONG(0x0);
        LONG(0x0);
        LONG(0x0);
        LONG(0x0);

        _dbg_data = .;
        FILL(0x0000);

        . = 0x400-4;
        LONG(0x0);
    } > dbgram

    .shmem :
    {
        _shm_sdr_base = .;
        LONG(0xFFFFFFFF);
        _shm_cmd = .;
        LONG(0x0);
		_shm_tsc_ready = .;
		LONG(0x0);
		_shm_tsc_non_zero = .;
		LONG(0x0);
		_shm_sec_overflow = .;
		LONG(0x0);
        _shm_sec_info = .; 
        FILL(0x00000000);

        . = 0x400 - 4;
        LONG(0x0);
    } > shmram
}
