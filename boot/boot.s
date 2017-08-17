;   定义一些常用的位常量
;   用在段选择子，描述符 ……中


;	下面的七个常量标注了内核加载的位置以及起始位置
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x2
PAGE_DIR_TABLE_POS equ 0x100000
PT_NULL equ 0x0
KERNEL_START_SECTOR equ 0x9
KERNEL_BIN_BASE_ADDR equ 0x70000
KERNEL_ENTER_POINT equ 0x1500

DESC_G_4K   equ     1_000_0000_0000_0000_0000_0000b
DESC_D_32   equ      1_00_0000_0000_0000_0000_0000b
DESC_L      equ       0_0_0000_0000_0000_0000_0000b

DESC_AVL    equ         0_0000_0000_0000_0000_0000b

DESC_LIMIT_CODE2    equ     1111_0000_0000_0000_0000b
DESC_LIMIT_DATA2    equ     DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2   equ     0000_0000_0000_0000_0000b

DESC_P      equ     1_000_0000_0000_0000b
DESC_DPL_0  equ     00_0_0000_0000_0000b
DESC_DPL_1  equ     01_0_0000_0000_0000b
DESC_DPL_2  equ     10_0_0000_0000_0000b
DESC_DPL_3  equ     11_0_0000_0000_0000b

DESC_S_CODE equ     1_0000_0000_0000b
DESC_S_DATA equ     DESC_S_CODE
DESC_S_sys  equ     0_0000_0000_0000b

DESC_TYPE_CODE  equ     1000_0000_0000b
DESC_TYPE_DATA  equ     0010_0000_0000b

DESC_CODE_HIGH4 equ     (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
                        DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + \
                        DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00

DESC_DATA_HIGH4 equ     (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
                        DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + \
                        DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00

DESC_VIDEO_HIGH4    equ     (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
                            DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + \
                            DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b


RPL0    equ     00b
RPL1    equ     01b
RPL2    equ     10b
RPL3    equ     11b
TI_GDT  equ     000b
TI_LDT  equ     100b


PG_P    equ     1b
PG_RW_R equ     00b
PG_RW_W equ     10b
PG_US_S equ     000b
PG_US_U equ     100b
