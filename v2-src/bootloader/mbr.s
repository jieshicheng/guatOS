%include "bootloader/constant.s"

; BIOS加电后将硬盘的第一个扇区文件读入到内存0x7c00位置处
; 并将CS，IP指向这个位置，然后开始执行代码，所以我们的引导程序也
; 编译成放在内存0x7c00处

SECTION MBR vstart=0x7c00 
    mov ax, cs    ; 将cs寄存器的值赋给其他的段寄存器
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00 ; ESP 栈指针寄存器指向同当前代码执行处
    mov ax, 0xb800 ; 0xb800 是显卡所在内存的位置。这里将显卡位置赋值给gs，可以向屏幕输出信息
    mov gs, ax

    mov ax, 0600h ; 通过中断调用BIOS的中断来使屏幕清空
    mov bx, 0700h
    mov cx, 0
    mov dx, 184fh
    int 10h

    mov eax, LOADER_START_SECTOR ; MBR的职责是加载内核加载器，这里指明内核加载器所在硬盘的扇区号
    mov bx, LOADER_BASE_ADDR ; 这里指明将内核加载器加载到内存的哪一个位置
    mov cx, 4
    call rd_disk_m_16 ; 开始加载内核加载器 

    jmp LOADER_BASE_ADDR ; 然后跳到相应内存，去执行内核加载器的命令

; 下面这个函数用来读取硬盘的数据到内存
; 大部分外部设备都是以端口（寄存器）来与我们打交道的
rd_disk_m_16:
    mov esi, eax
    mov di, cx

    mov dx, 0x1f2
    mov al, cl
    out dx, al
    mov eax, esi

    mov dx, 0x1f3
    out dx, al

    mov cl, 8
    shr eax, cl
    mov dx, 0x1f4
    out dx, al

    shr eax, cl
    mov dx, 0x1f5
    out dx, al

    shr eax, cl
    and al, 0x0f
    or al, 0xe0
    mov dx, 0x1f6
    out dx, al

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

.not_ready:
    nop
    in al, dx
    and al, 0x88
    cmp al, 0x08
    jnz .not_ready

    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax
    mov dx, 0x1f0

.go_on_read:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .go_on_read
    ret

    times 510 - ($ - $$) db 0 ; MBR的特征要求，结尾必须是0x55 0xAA
    db 0x55, 0xAA
