BUILD_DIR = ./build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB = -I lib/ -I kernel/ -I kernel/debug -I device/
ASFLAGS = -f elf
CFLAGS1 = $(LIB) -c -fno-builtin
CFLAGS2 = $(LIB) -c -fno-builtin -fno-stack-protector
LDFLAGS = -Ttext $(ENTRY_POINT) -e main
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/bitmap.o $(BUILD_DIR)/string.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/timer.o \
		$(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o $(BUILD_DIR)/debug.o

#####		c  complier		#######

$(BUILD_DIR)/main.o : kernel/main.c \
					  kernel/init.h \
					  lib/print.h \
					  lib/stdint.h \
					  kernel/debug/debug.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/init.o : kernel/init.c kernel/init.h \
					  lib/print.h \
					  kernel/interrupt.h \
					  device/timer.h \
					  lib/stdint.h \
					  kernel/memory.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/interrupt.o : kernel/interrupt.c kernel/interrupt.h \
						   lib/print.h \
						   lib/global.h \
						   lib/io.h \
						   lib/stdint.h
	$(CC) $(CFLAGS2) $< -o $@

$(BUILD_DIR)/timer.o : device/timer.c device/timer.h \
					   lib/stdint.h \
					   lib/io.h \
					   lib/print.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/debug.o : kernel/debug/debug.c kernel/debug/debug.h \
					   lib/print.h \
					   lib/stdint.h \
					   kernel/interrupt.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/bitmap.o : lib/bitmap.c lib/bitmap.h \
						lib/stdint.h \
						lib/string.h \
						lib/global.h kernel/debug/debug.h \
 						kernel/interrupt.h lib/print.h \
						lib/global.h kernel/debug/debug.h \
						kernel/interrupt.h lib/print.h \
						kernel/debug/debug.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/memory.o : kernel/memory.c kernel/memory.h \
						lib/bitmap.h \
						lib/stdint.h \
						lib/print.h \
						lib/string.h \
						lib/global.h \
						kernel/debug/debug.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/string.o : lib/string.c lib/string.h \
			lib/global.h \
			kernel/debug/debug.h \
			lib/stdint.h
	$(CC) $(CFLAGS1) $< -o $@

##### 		nasm complier   ########

$(BUILD_DIR)/kernel.o : kernel/kernel.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/print.o : lib/print.s
	$(AS) $(ASFLAGS) $< -o $@

#####		ld 		#########

$(BUILD_DIR)/kernel.bin : $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@


#####		product file 	#######

clean :
	cd $(BUILD_DIR) && rm -f ./*

build : $(BUILD_DIR)/kernel.bin 

