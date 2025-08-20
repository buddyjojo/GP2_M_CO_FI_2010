

MEMORY
{
/*
    coderam : ORIGIN = 0x00000000, LENGTH = 0x00002000
    dataram : ORIGIN = 0x00002000, LENGTH = 0x00001000
    dbgram  : ORIGIN = 0x00004000, LENGTH = 0x00000400
*/
    vectors : ORIGIN = 0x00000000, LENGTH = 0x00002000
    dataram : ORIGIN = 0x00002000, LENGTH = 0x00001000
    coderam : ORIGIN = 0x00003000, LENGTH = 0x00003000
    dbgram  : ORIGIN = 0x00006000, LENGTH = 0x00000400
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
    00004x00    +-------------------------------+
                |                               |
    00005000    +-------------------------------+
*/


    .vectors :
    {
        *(.vectors)
    } > vectors

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


}
