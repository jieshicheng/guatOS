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
		-Wmissing-prototypes -Wsystem-headers -fno-stack-protector"
LIB="-I ../lib/ -I  ../kernel/ -I ../device/ -I ../thread/ -I ../userprog/ \
		-I ../fileSystem/ -I ../shell/"
OBJS="../build/string.o ../build/syscall.o ../build/stdio.o ../build/debug.o \
	start.o ../build/print.o ../build/interrupt.o ../build/memory.o \
	../build/kernel.o ../build/sync.o ../build/thread.o ../build/syscall-init.o \
	../build/bitmap.o ../build/list.o ../build/fs.o ../build/direct.o \
	../build/file.o"

DD_IN=$BIN
DD_OUT="/home/jieshi/bochs-2.6/c.img"

nasm -f elf ./start.s -o ./start.o
ar rcs simple_ctr.a $OBJS start.o
gcc $CFLAGS $LIB -o $BIN".o" $BIN".c"
ld $BIN".o" simple_ctr.a -o $BIN
SEC_CNT=$(ls -l $BIN|awk '{printf("%d", ($5+511)/512)}')

dd if=./$DD_IN of=${DD_OUT} bs=512 count=$SEC_CNT seek=300 conv=notrunc
