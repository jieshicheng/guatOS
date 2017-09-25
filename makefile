BUILD_DIR = ./build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB = -I lib/ -I kernel/ -I kernel/debug -I device/ -I thread/ -I userprog/ -I fileSystem/
ASFLAGS = -f elf
CFLAGS1 = $(LIB) -c -fno-builtin
CFLAGS2 = $(LIB) -c -fno-builtin -fno-stack-protector
LDFLAGS = -Ttext $(ENTRY_POINT) -e main
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/fs.o $(BUILD_DIR)/file.o \
	   $(BUILD_DIR)/direct.o $(BUILD_DIR)/inode.o \
	   $(BUILD_DIR)/ide.o $(BUILD_DIR)/stdio-kernel.o \
	   $(BUILD_DIR)/stdio.o $(BUILD_DIR)/syscall-init.o \
	   $(BUILD_DIR)/syscall.o $(BUILD_DIR)/process.o $(BUILD_DIR)/tss.o \
	   $(BUILD_DIR)/ioqueue.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/console.o \
	   $(BUILD_DIR)/sync.o $(BUILD_DIR)/list.o $(BUILD_DIR)/thread.o \
	   $(BUILD_DIR)/switch.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/bitmap.o \
	   $(BUILD_DIR)/string.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/timer.o \
	   $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o $(BUILD_DIR)/debug.o

#####		c  complier		#######

$(BUILD_DIR)/main.o : kernel/main.c \
					  kernel/init.h \
					  lib/print.h \
					  lib/stdint.h \
					  kernel/debug/debug.h \
					  thread/thread.h \
					  kernel/memory.h \
					  kernel/interrupt.h \
					  device/console.h \
					  thread/sync.h \
					  userprog/process.h \
					  userprog/tss.h \
					  lib/syscall.h \
					  lib/stdio.h \
					  fileSystem/fs.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/init.o : kernel/init.c kernel/init.h \
					  lib/print.h \
					  kernel/interrupt.h \
					  device/timer.h \
					  lib/stdint.h \
					  kernel/memory.h \
					  thread/thread.h \
					  device/console.h \
					  thread/sync.h \
					  device/keyboard.h \
					  userprog/tss.h \
					  device/ide.h \
					  fileSystem/fs.h
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
					   lib/print.h \
					   kernel/interrupt.h \
					   thread/thread.h \
					   kernel/debug/debug.h \
					   lib/global.h
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
						kernel/debug/debug.h \
						thread/sync.h \
						thread/thread.h \
						lib/list.h \
						kernel/interrupt.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/string.o : lib/string.c lib/string.h \
						lib/global.h \
						kernel/debug/debug.h \
						lib/stdint.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/thread.o : thread/thread.c thread/thread.h \
						lib/stdint.h \
						lib/global.h \
						lib/string.h \
						kernel/memory.h \
						lib/list.h \
						kernel/interrupt.h \
						lib/print.h \
						kernel/debug/debug.h \
						userprog/process.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/list.o : lib/list.c lib/list.h \
					  lib/global.h \
					  kernel/interrupt.h \
					  lib/stdint.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/sync.o : thread/sync.c thread/sync.h \
					  lib/stdint.h \
					  lib/global.h \
					  kernel/debug/debug.h \
					  kernel/interrupt.h \
					  thread/thread.h \
					  lib/list.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/console.o : device/console.c device/console.h \
						 lib/stdint.h \
						 lib/global.h \
						 lib/print.h \
						 thread/thread.h \
						 thread/sync.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/keyboard.o : device/keyboard.c device/keyboard.h \
						  lib/stdint.h \
						  lib/global.h \
						  kernel/interrupt.h \
						  lib/print.h \
						  lib/io.h \
						  device/ioqueue.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/ioqueue.o : device/ioqueue.c device/ioqueue.h \
						 lib/stdint.h \
						 lib/global.h \
						 thread/thread.h \
						 thread/sync.h \
						 kernel/interrupt.h \
						 kernel/debug/debug.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/tss.o : userprog/tss.c userprog/tss.h \
					 lib/stdint.h \
					 lib/global.h \
					 lib/string.h \
					 lib/print.h
	$(CC) $(CFLAGS2) $< -o $@

$(BUILD_DIR)/process.o : userprog/process.c userprog/process.h \
						 lib/stdint.h\
						 lib/global.h \
						 userprog/tss.h \
						 kernel/memory.h \
						 thread/thread.h \
						 lib/bitmap.h \
						 device/console.h \
						 lib/string.h \
						 kernel/interrupt.h \
						 lib/list.h \
						 kernel/debug/debug.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/syscall.o : lib/syscall.c lib/syscall.h \
						 lib/stdint.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/syscall-init.o : userprog/syscall-init.c userprog/syscall-init.h \
							  thread/thread.h \
							  lib/syscall.h\
							  lib/stdint.h \
							  lib/print.h \
							  device/console.h \
							  lib/string.h \
							  kernel/memory.h
	$(CC) $(CFLAGS1) $< -o $@

$(BUILD_DIR)/stdio.o : lib/stdio.c lib/stdio.h \
					   lib/global.h lib/stdint.h \
					   lib/string.h lib/syscall.h
	$(CC) $(CFLAGS2) $< -o $@

$(BUILD_DIR)/stdio-kernel.o : kernel/stdio-kernel.c kernel/stdio-kernel.h \
							  device/console.h \
							  lib/stdio.h
	$(CC) $(CFLAGS2) $< -o $@

$(BUILD_DIR)/ide.o : device/ide.c device/ide.h \
					 lib/stdint.h lib/global.h \
					 lib/list.h lib/bitmap.h \
					 thread/sync.h \
					 kernel/debug/debug.h \
					 kernel/stdio-kernel.h \
					 lib/io.h \
					 device/timer.h \
					 kernel/interrupt.h \
					 lib/list.h \
					 lib/string.h
	$(CC) $(CFLAGS2) $< -o $@

$(BUILD_DIR)/fs.o : fileSystem/fs.c fileSystem/fs.h \
					lib/stdint.h lib/global.h \
					device/ide.h lib/list.h \
					kernel/debug/debug.h kernel/memory.h \
					kernel/stdio-kernel.h lib/string.h \
					fileSystem/direct.h fileSystem/inode.h fileSystem/super_block.h \
					fileSystem/file.h thread/thread.h
	$(CC) $(CFLAGS2) $< -o $@

$(BUILD_DIR)/direct.o : fileSystem/direct.c fileSystem/direct.h \
						device/ide.h lib/stdint.h lib/global.h \
						kernel/memory.h fileSystem/fs.h \
						kernel/debug/debug.h lib/string.h \
						fileSystem/inode.h fileSystem/file.h \
						fileSystem/super_block.h kernel/stdio-kernel.h
	$(CC) $(CFLAGS2) $< -o $@

$(BUILD_DIR)/file.o : fileSystem/file.c fileSystem/file.h \
					  lib/stdint.h thread/thread.h \
					  device/ide.h lib/bitmap.h \
					  kernel/memory.h fileSystem/inode.h \
					  lib/global.h kernel/stdio-kernel.h \
					  fileSystem/super_block.h lib/string.h \
					  fileSystem/direct.h kernel/interrupt.h kernel/debug/debug.h
	$(CC) $(CFLAGS2) $< -o $@

$(BUILD_DIR)/inode.o : fileSystem/inode.c fileSystem/inode.h \
					   lib/stdint.h lib/global.h \
					   lib/list.h device/ide.h lib/string.h \
					   thread/thread.h kernel/memory.h \
					   kernel/interrupt.h device/ide.h \
					   fileSystem/super_block.h kernel/debug/debug.h
	$(CC) $(CFLAGS2) $< -o $@


##### 		nasm complier   ########

$(BUILD_DIR)/kernel.o : kernel/kernel.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/print.o : lib/print.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/switch.o : thread/switch.s
	$(AS) $(ASFLAGS) $< -o $@

#####		ld 		#########

$(BUILD_DIR)/kernel.bin : $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@


#####		product file 	#######

clean :
	cd $(BUILD_DIR) && rm -f ./*

build : $(BUILD_DIR)/kernel.bin 

