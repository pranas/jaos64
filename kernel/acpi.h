#ifndef _ACPI_H
#define _ACPI_H

#include <stdint.h>

struct acpi_sdt_header {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
};
typedef struct acpi_sdt_header acpi_sdt_header;

struct rsdt_t {
  acpi_sdt_header h;
  uint32_t sdt_ptr [32];
};
typedef struct rsdt_t rsdt_t;

void apci_init();
void* get_rsdt();
void* find_sdt(void*, const char*);

#endif
