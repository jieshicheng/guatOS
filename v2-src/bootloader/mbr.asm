; guatOS
; module of master boot record
; author: linkcheng
; date: 2019 4.8

os_lba_add equ 100

SECTION code_mbr align=16 vstart=0x7c00
    
    ; print string
    ; set graph card memory
    mov ax, 0xb800
    mov es, ax

    ; set data segment
    mov ax, 0
    mov ds, ax

    ; set stack
    mov ax, 0
    mov ss, ax
    mov sp, ax

    ; start mbr process
    ; print message and loader os code.
    call print_message

    ; set ds and es equals os section
    mov ax, [cs:os_loader_add]
    mov dx, [cs:os_loader_add + 2]
    mov di, 16
    div di
    mov ds, ax
    mov es, ax

    ; loader os code
    call loader_os
    jmp far [0x04]


;------------------------------------------------------------;
; @brief loader kernal to memory
; @param
;------------------------------------------------------------;
loader_os:
    pusha
    push ds

    mov di, 0
    mov si, os_lba_add ; os code address in hardisk
    mov bx, 0

    call read_section
    
    ; get sections number of os
    mov dx, [0x2]
    mov ax, [0x0]
    mov bx, 512
    div bx
    cmp dx, 0
    jnz unfinished
    dec ax

    unfinished:
        cmp ax, 0
        jz finished
        
        mov si, os_lba_add + 1
        mov bx, 0
        mov cx, ax ; set loop times
    
    read_more:
        mov ax, ds ; every times when we read a section then set ds pointer to it's tail postion.
        add ax, 0x20
        mov ds, ax

        mov di, 0
        call read_section
        inc si
        loop read_more

    finished:
        pop ds ; reset ds to 0x1000
        mov ax, [0x6] ; kernal entry pointer's segment address. 32 bits
        mov dx, [0x8]
        call update_seg_base
        mov [0x6], ax ; update the correctly value.

        mov cx, [0xa]
        mov bx, 0xc
    
    fix_seg_add:
        mov ax, [bx]
        mov dx, [bx + 2]
        call update_seg_base
        mov [bx], ax
        add bx, 4
        loop fix_seg_add
               
    popa
    ret
;------------------------------------------------------------;


;------------------------------------------------------------;
; @brief recalculate the segment address according to the os_loader_add
; @param 
;------------------------------------------------------------;
update_seg_base:
    push dx

    add ax, [cs:os_loader_add]
    adc dx, [cs:os_loader_add + 0x2]
    shr ax, 4
    ror dx, 4
    and dx, 0xf000
    or ax, dx

    pop dx
    ret
;------------------------------------------------------------;

;------------------------------------------------------------;
; @brief read a section from hardisk
; @param si, bx: 28 bits sections start address stored in order
;        di: di means that beginning address of os.
;------------------------------------------------------------;
read_section:
    push ax
    push dx
    push cx

    mov dx, 0x1f2
    mov al, 1 ; read one section
    out dx, al

    inc dx
    mov ax, si
    out dx, al

    inc dx
    mov al, ah
    out dx, al

    inc dx
    mov al, bl
    out dx, al

    inc dx
    mov al, bh
    or al, 0xe0
    out dx, al

    inc dx
    mov al, 0x20
    out dx, al ; write read command

    wait_ready:
        in al, dx
        and al, 0x88
        cmp al, 0x08
        jnz wait_ready

    mov cx, 256
    mov dx, 0x1f0
    read_words:
        in ax, dx
        mov [di], ax
        add di, 2
        loop read_words
    
    pop cx
    pop dx
    pop ax
    ret
;------------------------------------------------------------;

;------------------------------------------------------------;
; @brief print message
; @param message 
;------------------------------------------------------------;
print_message:
    mov bx, message
    mov cx, pudding - message
    mov di, 0
    
    show:
        mov byte al, [bx]
        mov ah, 0x07
        mov [es:di], ax
        add di, 2
        inc bx
        loop show
    
    ret
;------------------------------------------------------------;

os_loader_add dd 0x10000

message db 'guatOS: Welcome to guatOS, It is mbr model now'

pudding: 
    times 510 - ($ - $$) db 0
    db 0x55, 0xAA
