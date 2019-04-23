#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

// REQUIREMENT: >= 8 digit filename
// REQUIREMENT: 3 digit file extension

// 32-byte entries
struct ENTRY {
  char filename[13];
  char file_extension[3];
  char file_size[8];
  char est_month[2];
  char est_day[2];
  char est_year[4];
};

// file allocation table (32-byte entries)
struct FAT {
  char reserved[6];
  char data[6];
  char next_block[6];
  char valid[1]; // Validity byte shown in class
  char filename[13];
};

struct NULL_BLOCK_ENTRY_DATA {
  char temporary[512];
};

struct NULL_BLOCK_DIR_DATA {
  char temporary[16];
};

FILE initialize_virtual_disk(char *virtual_disk_filename);
void initialize_reserved_blocks();

char *get_next_open_FAT_location();
char *get_next_open_RESERVED_location();
char *get_next_open_DATA_location();

#endif
