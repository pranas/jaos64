#include "acpi.h"

#include <string.h>

#include "memman.h"
#include "monitor.h"

void * madt;

void acpi_init()
{
	brute_create_page( 0x1FF0000, 0x1FF0000, 16, get_current_pml4(), 0);
	madt = find_sdt(get_rsdt(), "APIC");
	puts("[ACPI] MADT found at "); puthex(madt); puts("\n");
}

void * get_rsdt()
{
	char * i;
	for (i = 0xC0000; i < 0xFFFFF; i += 16)
	{
		if (strncmp(i, "RSD PTR ", 8))
		{
			puts("[ACPI] RSD PTR found at "); puthex(i); puts(" and it points to ");
			void * ptr = * (uint64_t*) (i + 16);
			puthex(ptr); puts("\n");
			return ptr;
		}
	}
}

void * find_sdt(void* rsdt_ptr, const char * signature)
{
	rsdt_t * rsdt = (rsdt_t*) rsdt_ptr;
	int n = ((rsdt->h).length - sizeof(rsdt->h)) / 4;
	int i;
	for (i = 0; i < n; i++)
	{
		acpi_sdt_header * h = (acpi_sdt_header*) (rsdt->sdt_ptr[i]);
		if (strncmp(h->signature, signature, 4))
			return (void*) h;
	}
	return NULL;
}
