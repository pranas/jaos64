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
