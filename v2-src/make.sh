`cp ./env/bochs/bochsrc ./output`
`cp ./env/bochs/guatos.img ./output`

`nasm ./bootloader/mbr.asm -o ./output/mbr.bin`
`nasm ./bootloader/loader.asm -o ./output/loader.bin`

`dd if=./output/mbr.bin of=./output/guatos.img bs=512 count=1 conv=notrunc`
`dd if=./output/loader.bin of=./output/guatos.img bs=512 seek=100 conv=notrunc`
