#include "diskio.h"

uint64_t read_sector(uint64_t lba, uint64_t address)
{
	return read_sectors(lba, address, 1);
}