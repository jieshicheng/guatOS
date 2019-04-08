`rm ../../output/*`

`nasm mbr.asm -o mbr.bin`

`dd if=./mbr.bin of=../../env/bochs/hd60.img bs=512 count=1 conv=notrunc`

`rm mbr.bin`

`cp ../../env/bochs/* ../../output`
