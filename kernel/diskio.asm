bits 64

global read_sectors

read_sectors:
; C passes parameters as follows:
;   rdi - lba
;   rsi - address
;   rdx - size
; Prepare those parameters for asm function
    push rdi
    push rsi
    pop rdi
    pop rax
    call read
    ret
    
;
; Procedure to read from disk
;   IN:
;       rax - LBA
;       rdi - destination
;       dl  - sector count
;       
;   OUT:
;       rdi - dest + copied mem
;       CF if error
;
read:
    push rdx
    push rcx

    mov ecx, eax

    mov al, dl          ; move sector count to al

    mov dx,1f2h         ;Sector count port
    ;mov al,1            ;Read one sector
    out dx,al

    mov al, cl		; ecx currently holds LBA
    inc edx			; port 1f3 -- LBAlow
    out dx, al

    mov al, ch
    inc edx			; port 1f4 -- LBAmid
    out dx, al

    bswap ecx
    mov al, ch		; bits 16 to 23 of LBA
    inc edx			; port 1f5 -- LBAhigh
    out dx, al

    mov al, cl            ; bits 24 to 28 of LBA
    or al, 0xe0 ;byte [esi + dd_sbits]    ; master/slave flag | 0xe0
    inc edx               ; port 1f6 -- drive select
    out dx, al

    inc edx			; port 1f7 -- command/status
    mov al, 0x20		; send "read" command to drive
    out dx, al

;7	BSY		Drive is preparing to accept/send data -- wait until this bit clears. If it never
;			clears, do a Software Reset. Technically, when BSY is set, the other bits in the
;			Status byte are meaningless.

; when BSY is clear, check

;3	DRQ		Set when the drive has PIO data to transfer, or is ready to accept PIO data.


still_going:
   in al,dx
   test al,8            ;This means the sector buffer requires servicing.
   jz still_going     ;Don't continue until the sector buffer is ready.
   mov rcx,512/2        ;One sector /2
   ;mov rdi, 0x10000 
   mov dx,1f0h         ;Data port - data comes in and out of here.
   rep insw
   
   mov rax, 1
   jnc goodEnd
   mov rax, 0
   
goodEnd:
   pop rcx
   pop rdx
   ret