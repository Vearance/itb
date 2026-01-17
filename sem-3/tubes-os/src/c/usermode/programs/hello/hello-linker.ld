ENTRY(_start)

SECTIONS {
    . = 0x00000000;

    .text ALIGN(4): {
        *crt0-hello.o(.text)
        *(.text)
    }

    .data ALIGN(4): {
        *(.data)
    }

    .bss ALIGN(4): {
        *(COMMON)
        *(.bss)
    }

    .rodata ALIGN(4): {
        *(.rodata*)
    }

    _linker_hello_end = .;
    ASSERT ((_linker_hello_end <= 1 * 1024 * 1024), "Error: User program linking result is more than 1 MiB")
}
