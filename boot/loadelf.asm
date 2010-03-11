bits 64

; INPUT:
;     RBX - contains ELF file to load address in memory
; OUTPUT:
;     RBX - contains entry point address

filestart dq 0x00000000
entrypoint dq 0x00000000

loadelf:
	mov [filestart], rbx
	mov rax, [rbx + off_elf_entry]
	mov [entrypoint], rax
	xor rax, rax
	xor rcx, rcx
	mov  cx, word [rbx + off_elf_phdrnr] ; get the number of program header entries
	add rbx, [rbx + off_elf_phdroffset]     ; rbx now points to program header pointer
	
@loop:                          ; parse each program header section
	mov eax, dword [rbx]              ; load type
	cmp eax, flag_loadable      ; check if its loadable
	jne @notload
	mov rsi, [filestart]
	add rsi, [rbx + off_phdr_offset] ; load address where to copy from
	mov rdi, [rbx + off_phdr_vaddr]    ; load address where to copy to
	push rcx                         ; i hope the stack works
	mov rcx, [rbx + off_phdr_filesz] ; get the size of the section
	cld                              ; make sure direction flag is 0
	rep movsb                        ; copy
	pop rcx                          ; lets hope rcx came back
@notload:
	add rbx, 0x30 ; 0x30 is probably the program header entry size :D
                  ; prepare to parse next program header entry if necessary
	loop @loop
	mov rbx, [entrypoint]
	ret           ; if we're lucky, we loaded an ELF file's image to memory

; flags
flag_loadable equ 0x01
; offset constants
off_elf_entry      equ 0x18
off_elf_phdroffset equ 0x20
off_elf_phdrnr     equ 0x38

off_phdr_type   equ 0x00
off_phdr_offset equ 0x08
off_phdr_vaddr  equ 0x10
off_phdr_filesz equ 0x20
