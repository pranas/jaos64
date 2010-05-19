#pragma once
#ifndef DISKIO_H
#define DISKIO_H

#include <stdint.h>

extern uint64_t read_sectors(uint64_t lba, uint64_t address, uint64_t size);

uint64_t read_sector(uint64_t lba, uint64_t address);

#endif