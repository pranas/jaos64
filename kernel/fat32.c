#include "fat32.h"
#include "memman.h"

void fat32_init()
{
	partition_table* table = palloc();
	fat32_volume_id* volume = ((uint64_t) table) + 512;
	
	puts("Initialising file system (using ");
	puthex((uint64_t) table);
	puts(" page)...\n");
	
	if (!table)
	{
		puts("Panic, no memory allocated!");
	}
	
	if (!read_sector(0, table))
	{
		puts("Panic, can't read disk!");
	}
	
	if (table->magic != 0xAA55)
	{
		puts("Warning! MBR probably damaged!");
	}
	
	puts("First partition located at ");
	putint(table->partition[0].lba);
	puts(" sector on disk\n");
	
	_partition_begin_lba = table->partition[0].lba;
	
	if (!read_sector(table->partition[0].lba, volume))
	{
		puts("Panic, can't read FAT32 volume id!");
	}
	
	volume->label[10] = 0;
	puts("Partition loaded... (Label=");
	puts(volume->label);
	puts(")\n");
	
	_partition = volume;
	_cluster_begin_lba = _partition_begin_lba + _partition->reserved_sectors + (_partition->number_of_fats * _partition->sectors_per_fat2);
	_fat_begin_lba = _partition_begin_lba + _partition->reserved_sectors;
	
	
}

uint64_t cluster2lba(uint64_t cluster)
{
	//LBA = cluster_begin_lba() + (cluster_number - 2) * sectors_per_cluster;
	return _cluster_begin_lba + ((cluster - 2) * _partition->sectors_per_cluster);
}

uint64_t find_next_cluster(uint64_t cluster)
{
	if (cluster >= 0x0ffffff8) return 0x0fffffff;
	
	uint32_t* fat = ((uint64_t) _partition) + 512;
	
	if (!read_sector(_fat_begin_lba + (cluster / _partition->bytes_per_sector / 4), fat))
	{
		puts("Error!");
	}
	
	return fat[cluster % _partition->bytes_per_sector / 4];
}

uint64_t put_dir(dir_entry* dir, int size)
{
	int i;
	puts("Files in dir:\n");
	for (i = 0; i < size; i++)
	{
		puts(dir[i].filename);
		puts("\n");
	}
}
