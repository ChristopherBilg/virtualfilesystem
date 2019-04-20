#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "main.h"

// Constants
#define VIRTUAL_DISK_FILE "vDisk" // The name of the file on actual disk
#define DISK_TOTAL_BLOCKS 20000 // can be changed
#define FAT_SP 0 // starting point
#define RESERVED_SP 1 // starting point
#define FAT_DATA_SP 3 // starting point
#define DISK_BLOCK_SIZE 512 // bytes
#define MAXIMUM_ENTRY_SIZE 256 // blocks

