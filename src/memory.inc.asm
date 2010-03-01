struc MemoryMapEntry
	.baseAddress	resq	1	; base address of address range
	.length			resq	1	; length of address range in bytes
	.type			resd	1	; type of address range
	.acpi_null		resd	1	; reserved
	; TODO:
	; 	Make sure to set that last dword to 1 before each call,
	; 	to make your map compatible with ACPI
endstruc

;---------------------------------------------
;	Get memory map from bios
;	/in es:di->destination buffer for entries
;	/ret bp=entry count
;---------------------------------------------

BiosGetMemoryMap:
	pushad
	xor	ebx, ebx
	xor	bp, bp					; number of entries stored here
	mov	edx, "PAMS"				; 'SMAP'
	mov	eax, 0xe820
	mov	ecx, 24					; memory map entry struct is 24 bytes
	int	0x15					; get first entry
	jc	.error	
	cmp	eax, "PAMS"				; bios returns SMAP in eax
	jne	.error
	test ebx, ebx				; if ebx=0 then list is one entry long; bail out
	je	.error
	jmp	.start
.next_entry:
	mov	edx, "PAMS"				; some bios's trash this register
	mov	ecx, 24					; entry is 24 bytes
	mov	eax, 0xe820
	int	0x15					; get next entry
.start:
	jcxz	.skip_entry			; if actual returned bytes is 0, skip entry
.notext:
	mov	ecx, [es:di + MemoryMapEntry.length]	; get length (low dword)
	test ecx, ecx				; if length is 0 skip it
	jne	short .good_entry
	mov	ecx, [es:di + MemoryMapEntry.length + 4]; get length (upper dword)
	jecxz .skip_entry			; if length is 0 skip it
.good_entry:
	inc	bp						; increment entry count
	add	di, 24					; point di to next entry in buffer
.skip_entry:
	cmp	ebx, 0					; if ebx return is 0, list is done
	jne	.next_entry				; get next entry
	jmp	.done
.error:
	stc
.done:
	popad
	ret