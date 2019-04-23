#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

// REQUIREMENT: >= 8 digit filename
// REQUIREMENT: 3 digit file extension

// 32-byte entries
struct ENTRY {
  char filename[23];
  char file_extension[3];
  char file_size[6];
};

// file allocation table (32-byte entries)
struct FAT {
  char reserved[6];
  char data[6];
  char next_block[6];
  char valid[1]; // Validity byte shown in class
  char filename[13];
};

#endif
