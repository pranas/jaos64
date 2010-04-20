;
; Procedure to load next cluster
;
;	OUT:
;		Carry if EOF

getNextCluster:
	pusha
	; check if we're already at EOF
	mov eax, [current_cluster]
	and eax, FAT_Cluster_Mask	; cluster mask
	cmp eax, FAT_EOF
	jge .eof 					; greater or equal
	
	; do the math to find out wich FAT sector to read
	xor edx, edx 				; div 32bit reg divides edx:eax
	xor ebx, ebx
	mov bx, [FAT32.bytes_per_sector]
	shr bx, 2 					; bx = bx / 4 (each entry is 32bits (4 B) long)
	div ebx
	
	push edx  					; div remainder
		
	; now read that sector
	; si points to Address packet
	
	add eax, [fat_begin_lba]
	mov [AddressPacket.lba], eax
	mov WORD [AddressPacket.sectors], 0x0001
	mov si, AddressPacket
	call readDisk

	; now let's do the math again to find out wich entry of FAT do we need
	; (use remainder)
	
	pop eax						; remainder	
	sal eax, 2					; eax = eax * 4
	
	mov esi, [AddressPacket.buffer]
	add esi, eax
	
	mov eax, [esi]				; next cluster
	mov [current_cluster], eax
	and eax, FAT_Cluster_Mask
	cmp eax, FAT_EOF
	jge .eof 					; greater or equal
	
	call clusterLBA
	mov [AddressPacket.lba], eax
	
	mov bl, [FAT32.sectors_per_cluster]
	mov [AddressPacket.sectors], bx
	
	mov si, AddressPacket
	call readDisk
	
	popa
	clc
	ret
.eof:
	popa
	stc	
	ret

;
; Procedure to readDisk using AddressPacket in si
;

readDisk:
	pusha
	mov ah, 0x42			; to read
	mov dl, [boot_drive]
	int 0x13
	popa
	ret
	
;
;	Procedure to get LBA from cluster number
;		IN:
;			eax - cluster number
;		OUT:
;			eax - LBA
;
;	LBA = cluster_begin_lba + (cluster_number - 2) * sectors_per_cluster;
;

clusterLBA:
	push ecx
	
	xor ecx, ecx
	dec eax
	dec eax
	mov cl, BYTE [FAT32.sectors_per_cluster]
	mul ecx
	add eax, [cluster_begin_lba]
	
	pop ecx
	ret
