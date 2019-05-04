BUILD_DIR = ./output
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
ASFLAGS = -f elf
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main
CFLAGS = -m32 -c
OBJS = $(BUILD_DIR)/main.o

#####		c  complier		#######

$(BUILD_DIR)/main.o : kernel/main.c
	$(CC) $(CFLAGS) $< -o $@

##### 		nasm complier   ########


#####		ld 		#########

$(BUILD_DIR)/kernel.bin : $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@


#####		product file 	#######

clean :
	cd $(BUILD_DIR) && rm -f ./*

build : $(BUILD_DIR)/kernel.bin 

