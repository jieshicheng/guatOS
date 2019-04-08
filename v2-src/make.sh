`cp ./env/bochs/bochsrc ./output`
`cp ./env/bochs/guatos.img ./output`

`nasm ./bootloader/mbr.asm -o ./output/mbr.bin`

`dd if=./output/mbr.bin of=./output/guatos.img bs=512 count=1 conv=notrunc`
