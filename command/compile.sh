#if [[ ! -d "../lib" || ! -d "../build" ]];then
#	echo "dependent dir don\'t exist!"
#	cwd=$(pwd)
#	cwd=${cwd##*/}
#	cwd=${cwd%/}
#	if [[ $cwd != "out_file" ]];then
#		echo -e "you\'d better in command dir\n"
#	fi
#	exit
#fi

BIN="cat"
CFLAGS="-Wall -c -fno-builtin -W -Wstrict-prototypes \
		-Wmissing-prototypes -Wsystem-headers"
LIB="-I ../lib/ -I  ../kernel/ -I ../device/ -I ../thread/ -I ../userprog/ \
		-I ../fileSystem/ -I ../shell/"
OBJS="$(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/wait_exit.o \
	   $(BUILD_DIR)/exec.o $(BUILD_DIR)/buildin_cmd.o \
	   $(BUILD_DIR)/shell.o $(BUILD_DIR)/fork.o \
	   $(BUILD_DIR)/fs.o $(BUILD_DIR)/file.o \
	   $(BUILD_DIR)/direct.o $(BUILD_DIR)/inode.o \
	   $(BUILD_DIR)/ide.o $(BUILD_DIR)/stdio-kernel.o \
	   $(BUILD_DIR)/stdio.o $(BUILD_DIR)/syscall-init.o \
	   $(BUILD_DIR)/syscall.o $(BUILD_DIR)/process.o $(BUILD_DIR)/tss.o \
	   $(BUILD_DIR)/ioqueue.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/console.o \
	   $(BUILD_DIR)/sync.o $(BUILD_DIR)/list.o $(BUILD_DIR)/thread.o \
	   $(BUILD_DIR)/switch.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/bitmap.o \
	   $(BUILD_DIR)/string.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/timer.o \
	   $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o $(BUILD_DIR)/debug.o"
DD_IN=$BIN
DD_OUT="/home/jieshi/bochs-2.6/c.img"

nasm -f elf ./start.s -o ./start.o
ar rcs simple_ctr.a $OBJS start.o
gcc $CFLAGS $LIB -o $BIN".o" $BIN".c"
ld $BIN".o" simple_ctr.a -o $BIN
SEC_CNT=$(ls -l $BIN|awk '{printf("%d", ($5+511)/512)}')

dd if=./$DD_IN of=${DD_OUT} bs=512 count=$SEC_CNT seek=300 conv=notrunc
