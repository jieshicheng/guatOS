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
        mov es, ax
        
        call get_memory_size
        jmp $

;----------------- get machine memory size ------------------;
;----------------- param none -------------------------------;
;----------------- return store in 0x10000, 32bits ----------;
    get_memory_size:
        pushad

        xor ebx, ebx
        mov edx, 0x534d4150 ; magic number
        mov di, 0
        .get_next_add:
            mov ecx, 20
            mov eax, 0xe820 ; interrupt number
            int 0x15
            jc .end_get_add
            add di, cx
            cmp ebx, 0
            jnz .get_next_add
        .end_get_add:
        ; get add number
            mov ax, di
            mov dx, 0
            mov bx, 20
            div bx

        ; bubbling_sort to get max memory size
            mov ebx, 0 ; max value of memory size
            mov cx, ax ; loop times
            mov di, 0
        .bubbling_sort:
            mov eax, [es:di]
            add eax, [es:di + 8]
            add di, 20
            cmp ebx, eax
            jge .next_value
            mov ebx, eax
        .next_value:
            loop .bubbling_sort
            jmp .finished_get_memory
        .finished_get_memory:
            mov [es:0x0], ebx

            popad
            ret

SECTION data_1 align=16 vstart=0
    times 1024 db 0

SECTION stack align=16 vstart=0
    times 512 db 0
    stack_end:

SECTION trail align=16
    program_end:
