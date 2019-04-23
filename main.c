#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "main.h"

// Constants
#define VIRTUAL_DISK_FILE "Drive5MB" // The name of the file on actual disk
#define DISK_TOTAL_BLOCKS 500 // can be changed
#define FAT_SP 0 // starting point
#define RESERVED_SP 1 // starting point
#define DATA_SP 3 // starting point
#define DISK_BLOCK_SIZE 512 // bytes
#define MAXIMUM_ENTRY_SIZE 64 // blocks
#define ENTRY_SIZE 32 // blocks
#define FAT_SIZE 32 // blocks

// Check if these are even used?
FILE *virtual_disk;
char temp1[3];
char temp2[5];
char temp_ptr[6];
unsigned int cwd_level = 0;

FILE initialize_virtual_disk(char *virtual_disk_filename) {
  virtual_disk = fopen(virtual_disk_filename, "r+");
  initialize_reserved_blocks();

  return *virtual_disk;
}

void initialize_reserved_blocks() {
  int index_ptr = 0;
  fseek(virtual_disk, 0, SEEK_SET);

  struct FAT fat = {};
  struct ENTRY root_dir = {};
  sprintf(temp_ptr, "%u", DISK_BLOCK_SIZE * RESERVED_SP / 17);
  strcpy(fat.filename, "/");
  strcpy(fat.valid, "1");
  strcpy(fat.reserved, temp_ptr);
  strcpy(fat.data, temp_ptr);
  strcpy(fat.next_block, "-1"); // -10 indicates the end of the block pointer "chain"
  strcpy(root_dir.filename, "/");
  strcpy(root_dir.file_extension, "dir");
  strcpy(root_dir.file_size, "0");

  // Now that the above FAT has been created, write it to the disk
  fwrite(&fat, sizeof(struct FAT), 1, virtual_disk);
  fseek(virtual_disk, DATA_SP * DISK_BLOCK_SIZE, SEEK_SET);
  fputc(' ', virtual_disk); // initialize the first data block in the FAT
  
  cwd_level = DATA_SP * DISK_BLOCK_SIZE;
  index_ptr = fseek(virtual_disk, RESERVED_SP * DISK_BLOCK_SIZE, SEEK_SET);

  // Now write the root directory to disk
  fwrite(&root_dir, sizeof(struct ENTRY), 1, virtual_disk);

  // Tell the user that their command was run successfully
  printf("%s> Reserved blocks have been initialized successfully", VIRTUAL_DISK_FILE);
}

int get_file_index(char *filename) {
  char temp_fn[13];
  int temp_index = 0;
  
  // if no file was found, then return -1
  return -1;
}

char *get_next_open_FAT_location() {
  char temporary[1];

  // reset the file pointer to the beginning of the FAT data
  int temp = RESERVED_SP * DISK_BLOCK_SIZE;
  fseek(virtual_disk, temp, SEEK_SET);

  // loop through FAT looking for next open location
  for (int index=0; index<temp; index+=FAT_SIZE) {
    fseek(virtual_disk, index, SEEK_SET);
    fread(temporary, 1, 1, virtual_disk); // read 1 byte from virtual dis
    if (strcmp(temporary, "") == 0) { // meaning temporary = "" (blank string)
      sprintf(temp_ptr, "%u", index / 17);
      return temp_ptr;
    }
  }

  // if no open location is found then return -1
  return "-1";
}

char *get_next_open_RESERVED_location() {
  char temporary[13]; // 13 bytes long filename, similiar to above function

  // reset the file pointer to the beginning of the RESERVED data
  int temp = RESERVED_SP * DISK_BLOCK_SIZE;
  fseek(virtual_disk, temp, SEEK_SET);

  // loop until we, hopefully, find an open location for new RESERVED data
  for (int index=0; index<temp; index+=ENTRY_SIZE) {
    fseek(virtual_disk, (temp + index), SEEK_SET);
    fread(temporary, 13, 1, virtual_disk); // read 13 bytes from virtual disk
    if (strcmp(temporary, "") == 0) { // meaning temporary = "" (blank string)
      int location = ((index / ENTRY_SIZE * 4) + (RESERVED_SP * DISK_BLOCK_SIZE / 16) + (1));
      sprintf(temp_ptr, "%u", location);
      return temp_ptr;
    }
  }

  // if no open location is found then return -1
  return "-1";
}

char *get_next_open_DATA_location() {
  char temporary[1]; // 1 bytes long for the valid member of the "struct ENTRY"

  // reset the file pointer to the beginning fo the DATA storage area
  int temp = DATA_SP * DISK_BLOCK_SIZE;
  fseek(virtual_disk, temp, SEEK_SET);

  // loop through all blocks of data entries until an open one is found
  for (int index=0; index < (DISK_TOTAL_BLOCKS - 5) * DISK_BLOCK_SIZE; index+=DISK_BLOCK_SIZE) {
    fseek(virtual_disk, temp + index, SEEK_SET);
    fread(temporary, 1, 1, virtual_disk); // read 1 byte from the disk file
    if (strcmp(temporary, "") == 0) { // meaning the block has not been used yet
      int location = ((index / DISK_BLOCK_SIZE * ENTRY_SIZE) + (DATA_SP * DISK_BLOCK_SIZE / 16) + (1));
      sprintf(temp_ptr, "%u", location);
      return temp_ptr;
    }
  }

  // if no open data blocks then return -1
  return "-1";
}
