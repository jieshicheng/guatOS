; guatOS
; author: linkcheng
; date: 2019 4.8

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
    jmp $

loader_os:
    mov di, 0
    mov cx, 100 ; os code address in hardisk
    mov bx, 0

    call read_section

    ret

; 28 bits sections start address store in cl, ch, bl and bh.
read_section:
    push ax
    push dx
    
    mov dx, 0x1f2
    mov al, 1 ; read one section
    out dx, al

    inc dx
    mov al, cl
    out dx, al

    inc dx
    mov al, ch
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
    
    pop dx
    pop ax
    ret

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

lba db 0, 0, 0, 0

os_loader_add dd 0x10000

message db 'guatOS: Welcome to guatOS, It is mbr model now'

pudding: 
    times 510 - ($ - $$) db 0
    db 0x55, 0xAA
