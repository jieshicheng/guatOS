; guatOS
; module of kernal
; author: linkcheng
; date: 2019 4.16

SECTION header align=16 vstart=0
    program_length dd program_end

    kernal_entry dw os_start
                 dd section.code_1.start

    segment_number dw (header_end - code_1_segment) / 4

    code_1_segment dd section.code_1.start
    data_1_segment dd section.data_1.start
    stack_segment dd section.stack.start
    header_end:

SECTION code_1 align=16 vstart=0
    os_start:
        ; set ss sp
        mov ax, [stack_segment]
        mov ss, ax
        mov sp, stack_end

        ; set ds es
        mov ax, [data_1_segment]
        mov ds, ax
        mov ax, 0xb800
        mov es, ax

        jmp $

SECTION data_1 align=16 vstart=0
    message db 'welcome to guatOS, this is kernal now'
            db 0x0d, 0x0a
            db 'version: 1.0'
            db 0x0d, 0x0a
            db 'author: linkcheng'

SECTION stack align=16 vstart=0
    resb 512
    stack_end:

SECTION trail align=16
    program_end:
