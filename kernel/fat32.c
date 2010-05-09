// #include <string.h>
#include "fat32.h"
#include "memman.h"
#include <string.h>

void fat32_init()
{
	partition_table* table = alloc_kernel_page(1);
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
	
	// void* buffer = alloc_kernel_page(((_partition->sectors_per_cluster * _partition->bytes_per_sector) / 4096) + 1);
	
	// read_cluster(_partition->root_cluster, buffer);
	// puthex((uint64_t) buffer);
	// put_dir(buffer, 10);
	
	// puts("Search returned: ");
	// puthex(find_file("TST7132   \0"));
	// puts("\n");
	
}

uint64_t read_cluster(uint64_t cluster, void* address)
{
	// puts("Read ");
	// putint(cluster);
	// puts(" cluster (");
	// putint(_partition->sectors_per_cluster);
	// puts(" sectors) at ");
	// puthex((uint64_t) address);
	// puts("\n");
	
	return read_sectors(cluster2lba(cluster), address, _partition->sectors_per_cluster);
}

void read_file(uint64_t cluster, void* address)
{
	read_cluster(cluster, address);
	
	while ((cluster = find_next_cluster(cluster)) < 0x0ffffff8)
	{
		address = (uint64_t) address + (_partition->sectors_per_cluster * _partition->bytes_per_sector);
		read_cluster(cluster, address);
	}
}

dir_entry* find_file(char* name)
{
	dir_entry* list = alloc_kernel_page(((_partition->sectors_per_cluster * _partition->bytes_per_sector) / 4096) + 1);
	
	uint64_t cluster = _partition->root_cluster;
	int entries_per_cluster = (_partition->sectors_per_cluster * _partition->bytes_per_sector) / 32;
	do
	{
		read_cluster(cluster, list);
		
		int i;
		
		for (i = 0; i < entries_per_cluster; i++)
		{
			list[i].filename[10] = 0;
			if (strncmp(list[i].filename, name, strlen(name)-1))
			{
				dir_entry file = list[i];
				free_kernel_page(list);
				return &file;//.cluster_high * 0x100 + list[i].cluster_low;
			}
		}
		
		// puts("Next cluster to read:");
		// puthex(find_next_cluster(cluster));
		// puts("\n");
	} while ((cluster = find_next_cluster(cluster)) < 0x0ffffff8);
	
	free_kernel_page(list);
	
	return 0;
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
	
	//puthex(fat);
	
	if (!read_sector(_fat_begin_lba + (cluster / _partition->bytes_per_sector / 4), fat))
	{
		puts("Error!");
	}
	
	return fat[cluster % _partition->bytes_per_sector];
}

uint64_t put_dir(dir_entry* dir, int size)
{
	int i;
	puts("Files in dir:\n");
	for (i = 0; i < size; i++)
	{
		if ((dir[i].attribute != 0x0F) || (dir[i].attribute != 0x10)) // no long names and dirs
		{
			puts(dir[i].filename);
			puts("\n");
		}
	}
}
