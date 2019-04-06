%include "boot.s"

section loader vstart=LOADER_BASE_ADDR
    LOADER_STACK_TOP equ LOADER_BASE_ADDR
    jmp loader_start

; 这里定义GDT
; GDT的第一个不可用，设置为全0
GDT_BASE:
    dd 0x00000000
    dd 0x00000000
; 代码段
CODE_DESC:
    dd 0x0000ffff
    dd DESC_CODE_HIGH4
; 数据段
DATA_STACK_DESC:
    dd 0x0000ffff
    dd DESC_DATA_HIGH4
; 显卡段
VIDEO_DESC:
    dd 0x80000007
    dd DESC_VIDEO_HIGH4

; 得到GDT的大小和地址
    GDT_SIZE equ ($ - GDT_BASE)
    GDT_LIMIT equ (GDT_SIZE - 1)
; 预留60个位置给以后使用
    times 60 dq 0
; 定义三个段选择子
    SELECTOR_CODE equ ((0x0001 << 3) + TI_GDT + RPL0)
    SELECTOR_DATA equ ((0x0002 << 3) + TI_GDT + RPL0)
    SELECTOR_VIDEO equ ((0x0003 << 3) + TI_GDT + RPL0)
; 定义GDT指针，用来在GDT寄存器中加载它
    gdt_ptr dw GDT_LIMIT
            dd GDT_BASE


loader_start:
    mov sp, LOADER_BASE_ADDR ; 设置栈顶

; 以下是开启保护模式的流程
    in al, 0x92
    or al, 0000_0010B
    out 0x92, al

    lgdt [gdt_ptr]

    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax
; 保护模式打开，跳转过去执行，这里有个强制刷新流水线的功能
    jmp dword SELECTOR_CODE:p_mode_start


[bits 32]
p_mode_start:
    mov ax, SELECTOR_DATA ; 令段寄存器指向数据段
    mov ds, ax
    mov es, ax
    mov ss, ax
    
    mov esp, LOADER_STACK_TOP ; 设置栈顶
    mov ax, SELECTOR_VIDEO ; gs指向显卡段，用来输出信息
    mov gs, ax
    
; 接下来就是loader的主要任务了，将内核文件加载到内存中来
    mov eax, KERNEL_START_SECTOR
    mov ebx, KERNEL_BIN_BASE_ADDR
    mov ecx, 200
; 这里是读取内核文件
    call rd_disk_m_32
; 然后建立分页机制
    call setup_page
; 分页之后对GDT也要有所修改，令GDT指向虚拟地址
    sgdt [gdt_ptr]
    mov ebx, [gdt_ptr + 2]
    or dword [ebx + 0x18 + 0x4], 0xc0000000
    add dword [gdt_ptr + 2], 0xc0000000
    add esp, 0xc0000000
    mov eax, PAGE_DIR_TABLE_POS
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
; 修改完成，重新加载
    lgdt [gdt_ptr]
; 跳到内核文件处去执行内核
    jmp SELECTOR_CODE:enter_kernel


; 由于我们加载进来的还只是内核文件，我们要对这个文件进行解析
enter_kernel:
; 解析文件
    call kernel_init
    mov esp, 0xc009f000 ; 设置栈顶
    jmp KERNEL_ENTER_POINT ; 解析完毕，内核指令已经放在0x1500处了，跳过去执行即可

; 建立分页机制，是二级目录机制，这个目录放在0x100000 也就是物理内存1M以后的位置
setup_page:
    mov ecx, 4096
    mov esi, 0
.clear_page_dir:
    mov byte [PAGE_DIR_TABLE_POS + esi], 0
    inc esi
    loop .clear_page_dir

.create_pde:
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x1000
    mov ebx, eax
    or eax, PG_US_U | PG_RW_W | PG_P
    mov [PAGE_DIR_TABLE_POS + 0x0], eax
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax
    sub eax, 0x1000
    mov [PAGE_DIR_TABLE_POS + 4092], eax
    
    
    mov ecx, 256
    mov esi, 0
    mov edx, PG_US_U | PG_RW_W | PG_P
.create_pte:
    mov [ebx + esi * 4], edx
    add edx, 4096
    inc esi
    loop .create_pte

    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x2000
    or eax, PG_US_U | PG_RW_W | PG_P
    mov ebx, PAGE_DIR_TABLE_POS
    mov ecx, 254
    mov esi, 769
.create_kernel_pde:
    mov [ebx + esi * 4], eax
    inc esi
    add eax, 0x1000
    loop .create_kernel_pde
    ret

; 内核文件解析，展开内核文件根据ELF文件格式，然后将可执行代码放到0x1500处
kernel_init:
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    mov dx, [KERNEL_BIN_BASE_ADDR + 42]
    mov ebx,  [KERNEL_BIN_BASE_ADDR + 28]
    add ebx, KERNEL_BIN_BASE_ADDR
    mov cx, [KERNEL_BIN_BASE_ADDR + 44]

.each_segment:
    cmp byte [ebx + 0], PT_NULL
    je .PTNULL
    push dword [ebx + 16]
    mov eax, [ebx + 4]
    add eax, KERNEL_BIN_BASE_ADDR
    push eax
    push dword [ebx + 8]
    call mem_cpy
    add esp, 12
.PTNULL:
    add ebx, edx
    loop .each_segment
    ret

mem_cpy:
    cld
    push ebp
    mov ebp, esp
    push ecx
    mov edi, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]
    rep movsb

    pop ecx
    pop ebp
    ret

rd_disk_m_32:
    mov esi, eax
    mov di, cx
    
    mov dx, 0x01f2
    mov al, cl
    out dx, al
    mov eax, esi

    mov dx, 0x01f3
    out dx, al

    mov cl, 8
    shr eax, cl
    mov dx, 0x01f4
    out dx, al

    shr eax, cl
    mov dx, 0x01f5
    out dx, al

    shr eax, cl
    and al, 0x0f
    or al, 0xe0
    mov dx, 0x01f6
    out dx, al

    mov dx, 0x01f7
    mov al, 0x20
    out dx, al

.not_ready:
    nop
    in al, dx
    and al, 0x88
    cmp al, 0x08
    jnz .not_ready

    mov ax, di
    mov dx, 128
    mul dx
    mov cx, ax
    mov dx, 0x1f0

.go_on_read:
    in ax, dx
    mov [ebx], ax
    add ebx, 2
    loop .go_on_read
    ret
