; guatOS
; author: linkcheng
; date: 2019 4.8

;org 0x7c00

; print string "hello world"
; set graph card memory
mov ax, 0xb800
mov es, ax

mov byte [es:0x00], 'H'
mov byte [es:0x01], 0x07
mov byte [es:0x02], 'e'
mov byte [es:0x03], 0x07
mov byte [es:0x04], 'l'
mov byte [es:0x05], 0x07
mov byte [es:0x06], 'l'
mov byte [es:0x07], 0x07
mov byte [es:0x08], 'o'
mov byte [es:0x09], 0x07
mov byte [es:0x0A], ' '
mov byte [es:0x0B], 0x07
mov byte [es:0x0C], 'W'
mov byte [es:0x0D], 0x07
mov byte [es:0x0E], 'o'
mov byte [es:0x0F], 0x07
mov byte [es:0x10], 'r'
mov byte [es:0x11], 0x07
mov byte [es:0x12], 'l'
mov byte [es:0x13], 0x07
mov byte [es:0x14], 'd'
mov byte [es:0x15], 0x07
mov byte [es:0x16], ' '
mov byte [es:0x17], 0x07

; set data segment
mov ax, cs
mov ds, ax

; set dividsor
mov ax, pudding
mov bx, 10

; start divided
mov dx, 0
div bx
mov [0x7c00 + vec + 0x00], dl

mov dx, 0
div bx
mov [0x7c00 + vec + 0x01], dl

mov dx, 0
div bx
mov [0x7c00 + vec + 0x02], dl

mov dx, 0
div bx
mov [0x7c00 + vec + 0x03], dl

mov dx, 0
div bx
mov [0x7c00 + vec + 0x04], dl

; print the address
mov al, [0x7c00 + vec + 0x04]
add al, 0x30
mov [es:0x18], al
mov byte [es:0x19], 0x07

mov al, [0x7c00 + vec + 0x04]
add al, 0x30
mov [es:0x1A], al
mov byte [es:0x1B], 0x07

mov al, [0x7c00 + vec + 0x02]
add al, 0x30
mov [es:0x1C], al
mov byte [es:0x1D], 0x07

mov al, [0x7c00 + vec + 0x01]
add al, 0x30
mov [es:0x1E], al
mov byte [es:0x1F], 0x07

mov al, [0x7c00 + vec + 0x00]
add al, 0x30
mov [es:0x20], al
mov byte [es:0x21], 0x07

infi:
    jmp near infi 

vec db 0, 0, 0, 0, 0

times 510 - ($ - $$) db 0
pudding db 0x55, 0xAA
