#include "elf.h"

int elf_check(void* *src) {
	if (((Elf64_Ehdr*) src)->e_ident[0] != 0x7F) return 1;
   	if (((Elf64_Ehdr*) src)->e_ident[1] != 'E') return 1;
   	if (((Elf64_Ehdr*) src)->e_ident[2] != 'L') return 1;
   	if (((Elf64_Ehdr*) src)->e_ident[3] != 'F') return 1;
   	if (((Elf64_Ehdr*) src)->e_type != 2) return 1;	// executable?
   	if (((Elf64_Ehdr*) src)->e_machine != 62) return 1;	// x86-64?
   	if (((Elf64_Ehdr*) src)->e_version == 0) return 1;
   	return 0;
}

void* load_executable(char* filename)
{
	// find file
	dir_entry* file = find_file(filename);

	// not found?
	if (!file) return 0;
	
	// calculate size and prepare buffer in kernel
    void* buffer = kmalloc(file->size);
    
	// read to buffer
	read_file(file->cluster_high * 0x100 + file->cluster_low, buffer);
	
	// is it elf?
	if (elf_check(buffer)) return 0;
	
	// use program header to load
	Elf64_Phdr* ph = buffer + ((Elf64_Ehdr *) buffer)->e_phoff;
	uint64_t i;
	for (i = 0; i < ((Elf64_Ehdr *) buffer)->e_phnum; i++)
	{
		if (ph[i].p_type == 1)
		{
			if (!alloc_page(ph[i].p_vaddr, ph[i].p_memsz / MEM_BLOCK_SIZE)) return 0;
			memcpy(ph[i].p_vaddr, buffer + ph[i].p_offset, ph[i].p_filesz);
		}
	}
	
	return (void*) ((uint64_t) ((Elf64_Ehdr *) buffer)->e_entry);
}

void debug_elf_header(Elf64_Ehdr* h)
{
	puts("Identification: ");
	puts(h->e_ident);
	puts("\n");
	puts("Type: ");
	puthex(h->e_type);
	puts("\n");
	puts("Machine: ");
	puthex(h->e_machine);
	puts("\n");
	puts("Version: ");
	puthex(h->e_version);
	puts("\n");
	puts("Entry point");
	puthex(h->e_entry);
	puts("\n");
	puts("Program header offset: ");
	puthex(h->e_phoff);
	puts("\n");
	puts("Section header offset: ");
	puthex(h->e_shoff);
	puts("\n");
	puts("Flags: ");
	puthex(h->e_flags);
	puts("\n");
	puts("ELF header size: ");
	puthex(h->e_ehsize);
	puts("\n");
	puts("Size of program header entry: ");
	puthex(h->e_phentsize);
	puts("\n");
	puts("Number of program header entries: ");
	puthex(h->e_phnum);
	puts("\n");
	puts("Size of section header entries: ");
	puthex(h->e_shentsize);
	puts("\n");
	puts("Number of section header entries: ");
	puthex(h->e_shnum);
	puts("\n");
	puts("Section name string table index: ");
	puthex(h->e_shstrndx);
	puts("\n");
}

void debug_program_header(Elf64_Phdr* h)
{
	puts("-- Program header --\n");
	puts("Type: ");
	puthex(h->p_type);
	puts("\n");
	puts("Flags: ");
	puthex(h->p_flags);
	puts("\n");
	puts("Offset in file: ");
	puthex(h->p_offset);
	puts("\n");
	puts("Virtual address: ");
	puthex(h->p_vaddr);
	puts("\n");
	puts("Reserved: ");
	puthex(h->p_paddr);
	puts("\n");
	puts("Segment size in file: ");
	puthex(h->p_filesz);
	puts("\n");
	puts("Segment size in mem: ");
	puthex(h->p_memsz);
	puts("\n");
	puts("Alignment: ");
	puthex(h->p_align);
	puts("\n");
}

void debug_section_header(Elf64_Shdr* h)
{
	puts("-- Section header --\n");
	puts("Name: ");
	puthex(h->sh_name);
	puts("\n");
	puts("Type: ");
	puthex(h->sh_type);
	puts("\n");
	puts("Flags: ");
	puthex(h->sh_flags);
	puts("\n");
	puts("Virtual address: ");
	puthex(h->sh_vaddr);
	puts("\n");
	puts("Offset in file: ");
	puthex(h->sh_offset);
	puts("\n");
	puts("Size of section: ");
	puthex(h->sh_size);
	puts("\n");
	puts("Link: ");
	puthex(h->sh_link);
	puts("\n");
	puts("Misc info: ");
	puthex(h->sh_info);
	puts("\n");
	puts("Alignment: ");
	puthex(h->sh_addralign);
	puts("\n");
	puts("Size of entries in section table: ");
	puthex(h->sh_entsize);
	puts("\n");
}