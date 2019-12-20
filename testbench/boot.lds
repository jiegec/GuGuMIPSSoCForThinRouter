SECTIONS {
    . = 0x80000000;
    .text 0x80000000 : {
        *(.text.init)
        *(.text)
    }
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    .got : {
        *(.got)
    }
    .data : {
        *(.data)
    }
    .physical : {
        *(.physical)
    }
    _bss_start = .;
    .bss : {
        *(.bss)
    }
    _bss_end = .;
}