%define buffer 0x10000

;
; Procedure to load complete file from fat32
;   IN:
;       rax - cluster of file to load
;       rdi - where to load
;
loadFile:
    push rax
    push rdx
    
    push rax
    call cluster2lba
    mov dl, [FAT32.sectors_per_cluster]
    ;mov rdi, 0x100000
    call read

; load more if there is

oneMoreCluster:   
    pop rax
    call getNextCluster
    jc loaded           ; if eof
    push rax
    call cluster2lba
    call read
    jmp oneMoreCluster

loaded:
    ; we loaded our file completely
    pop rdx
    pop rax
    clc
    ret


;
; Procedure to travel through FAT32 root directory and look for specified file
;   IN:
;       fileName - address to filename to look for
;   OUT:
;       rax - first cluster of file
;       
;       Carry if not found
;
searchFile:
    xor rax, rax
    mov eax, [FAT32.root_cluster]
readCluster:
    push rax ; save current cluster
    call cluster2lba

    mov dl, [FAT32.sectors_per_cluster]
    mov rdi, buffer
    call read
    
searchCluster:
	mov rax, buffer-32
	movzx rbx, WORD [bytes_per_cluster]
	add rbx, buffer

nextEntry:
	add rax, 32
	cmp rax, rbx		; bx = end of cluster in memory
	je notFound

	mov rdi, fileName
	mov rsi, rax
	mov rcx, 11
	; compare names
	repe cmpsb
	; if not the same, try next entry
	jnz nextEntry

	; file found

	; File entry looks like this:
	;
	; Filename 		times 11 db
	; Attribute 	db
	; ____ 			dq
	; Cluster High 	dw
	; ____ 			dd
	; Cluster Low 	dw
	; Size 			dd
    ; si is already +11 in entry
    movzx rax, WORD [rsi+9]
    ; TODO: fix this weird stuff with multiplication
    mov rbx, 0x100
    mul rbx
    mul rbx
    mov ax, [rsi+15]
    
    pop rdx ; islyginam stacka
    jmp finish

nextRoot:
    pop rax ; current cluster
    ; call a procedure to get next cluster
    call getNextCluster
    jnc readCluster ; if not end of root dir

notFound:
    stc
finish:
    ret

;
; Procedure to find next cluster
;   IN:
;       rax - current cluster
;
;	OUT:
;       rax - next cluster
;
;		Carry if EOF

getNextCluster:
    push rdx
    push rbx
    push rdi
    
	; check if we're already at EOF
	and eax, FAT_Cluster_Mask	; cluster mask
	cmp eax, FAT_EOF
	jge .eof 					; greater or equal
	
	; do the math to find out wich FAT sector to read
	xor edx, edx 				; div 32bit reg divides edx:eax
	xor ebx, ebx
	mov bx, [FAT32.bytes_per_sector]
	shr bx, 2 					; bx = bx / 4 (each entry is 32bits (4 B) long)
	div ebx
	
	push rdx  					; div remainder
		
	; now read that sector
	add eax, [fat_begin_lba]
	mov dl, 1
    mov rdi, 0x10000
    call read

	; now let's do the math again to find out wich entry of FAT do we need
	; (use remainder)
	
	pop rdx						; remainder
	sal edx, 2					; edx = edx * 4
	
	mov edi, 0x10000 ; buffer
	add edi, edx
	
	mov eax, [edi]			; next cluster

	and eax, FAT_Cluster_Mask
	cmp eax, FAT_EOF
	jge .eof 					; greater or equal
	
    pop rdi
    pop rbx
    pop rdx	
	clc
	ret
.eof:
    pop rdi
    pop rbx
    pop rdx
	stc	
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
    push rax
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

   pop rcx
   pop rdx
   pop rax
   ret
   
;
; Procedure to get LBA from cluster number
;	IN:
;		eax - cluster number
;	OUT:
;		eax - LBA
;
;	LBA = cluster_begin_lba + (cluster_number - 2) * sectors_per_cluster;
;

cluster2lba:
	push rcx
	
	xor ecx, ecx
	dec eax
	dec eax
	mov cl, BYTE [FAT32.sectors_per_cluster]
	mul ecx
	add eax, [cluster_begin_lba]
	
	pop rcx
	ret