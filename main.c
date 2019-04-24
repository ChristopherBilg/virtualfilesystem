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
#define PLACEHOLDER ' '

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
  sprintf(temp_ptr, "%u", DISK_BLOCK_SIZE * RESERVED_SP / 16 + 1);
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
  fputc(PLACEHOLDER, virtual_disk); // initialize the first data block in the FAT
  
  cwd_level = DATA_SP * DISK_BLOCK_SIZE;
  index_ptr = fseek(virtual_disk, RESERVED_SP * DISK_BLOCK_SIZE, SEEK_SET);

  // Now write the root directory to disk
  fwrite(&root_dir, sizeof(struct ENTRY), 1, virtual_disk);
}

int create_file(char *filename, char *ext) {
  char *indices[4];

  // First task is to find a free block for FAT, RESERVED, and DATA
  indices[1] = strdup(get_next_open_FAT_location());
  indices[2] = strdup(get_next_open_RESERVED_location());
  indices[3] = strdup(get_next_open_DATA_location());

  // if any of the above return -1, then error out
  if (strcmp(indices[1], "-1") == 0 ||
      strcmp(indices[2], "-1") == 0 ||
      strcmp(indices[2], "-1") == 0) {
    printf("An error occurred creating the file \"%s\"", filename);
    return -1;
  }

  // Write FAT entry
  struct FAT file = {};
  strcpy(file.valid, "1");
	strcpy(file.filename, filename);
	strcpy(file.reserved, indices[2]);
	strcpy(file.data, indices[3]);
	strcpy(file.next_block, "-1");
  fseek(virtual_disk, atoi(indices[1] - 1) * 16, SEEK_SET);
  fwrite(&file, sizeof(struct FAT), 1, virtual_disk);

  // Write RESERVED entry
  struct ENTRY entry = {};
  strcpy(entry.filename, filename);
  strcpy(entry.file_extension, ext);
  strcpy(entry.file_size, "0");
  fseek(virtual_disk, atoi(indices[2] - 1) * 16, SEEK_SET);
  fwrite(&entry, sizeof(struct ENTRY), 1, virtual_disk);

  // Write DATA entry (for this project, this will just show that data blocks are in use by a file)
  fseek(virtual_disk, atoi(indices[3] - 1) * 16, SEEK_SET);
  fputc(PLACEHOLDER, virtual_disk);

  // If the ext == "dir", then change the global current directory to this
  if (strcmp(ext, "dir") == 0)
    cwd_level = atoi(indices[3] - 1) * 16;

  // Add the file into the FAT directory structure
  for (int index = 0; index<FAT_SIZE; index++) {
    char validity_byte[1];
    fseek(virtual_disk, cwd_level + (index * 16), SEEK_SET);
    fread(validity_byte, 1, 1, virtual_disk);
    if (strcmp(validity_byte, "") == 0) { // if true then block is empty
      fseek(virtual_disk, -1, SEEK_CUR); // move back 1 byte
      fwrite(filename, 13, 1, virtual_disk);
      break;
    }
  }

  // return successfully
  return 1;
}

int delete_file(char *filename) {
  char temp1[6];
  char temp2[6];
  int file_location = 0;

  // check if file even exists
  if ((file_location = get_file_index(filename)) == -1)
    return -1;

  // Remove the file from the FAT directory structure
  for (int index=0; index<FAT_SIZE; index++) {
    char name[13] = {"\0"};
    fseek(virtual_disk, (index * 16) + cwd_level, SEEK_SET);
    fread(name, 1, 13, virtual_disk);
    if (strcmp(filename, name) == 0) {
      struct NULL_BLOCK_DIR_DATA null_block0= {""};
      fseek(virtual_disk, (index * 16) + cwd_level, SEEK_SET);
      fwrite(&null_block0, sizeof(struct NULL_BLOCK_DIR_DATA), 1, virtual_disk);
    }
  }
  
  // now that we know the file exists, clear the FAT, RESERVED, and DATA blocks
  fseek(virtual_disk, (file_location - 1) * 16, SEEK_SET);
	fseek(virtual_disk, 19, SEEK_CUR); // offset to DATA
	fread(temp1, 1, 6, virtual_disk);
	fseek(virtual_disk, (atoi(temp1) - 1) * 16, SEEK_SET);
	struct NULL_BLOCK_ENTRY_DATA null_block1 = {""};
	fwrite(&null_block1, sizeof(struct NULL_BLOCK_ENTRY_DATA), 1, virtual_disk);
  
  fseek(virtual_disk, (file_location - 1) * 16, SEEK_SET);
	fseek(virtual_disk, 13, SEEK_CUR); // offset to RESERVED
	fread(temp2, 1, 6, virtual_disk);
	fseek(virtual_disk, (atoi(temp2) - 1) * 16, SEEK_SET);
	struct ENTRY null_block2 = {};
	fwrite(&null_block2, sizeof(struct ENTRY), 1, virtual_disk);

  fseek(virtual_disk, (file_location - 1) * 16, SEEK_SET);
	struct FAT null_block3 = {};
	fwrite(&null_block3, sizeof(struct FAT), 1, virtual_disk);

  // return successfully
  return 1;
}

int read_from_file(char *filename) {
  // check that file exists
  int file_location = get_file_index(filename);
  if (file_location == -1)
    return -1;

  char data[DISK_BLOCK_SIZE];
  char temp_ptr[6];
  int temp_ptr2;

  fseek(virtual_disk, (file_location - 1) * 16, SEEK_SET);
	fseek(virtual_disk, 19, SEEK_CUR); // offset of 19 until internal ptr.
	fread(temp_ptr , 1, 6, virtual_disk); // internal pointer length of 6
	temp_ptr2 = atoi(temp_ptr);
	fseek(virtual_disk, (temp_ptr2 - 1) * 16, SEEK_SET);
	fread(data, 1, DISK_BLOCK_SIZE, virtual_disk);

  // write all data to console
	for (int i = 0; i < DISK_BLOCK_SIZE; i++)
		printf("%c", data[i]);
	printf("\n");
  
  return 1;
}

int write_to_file(char *filename, char *new_data) {
  // check that the file exists
  int file_location = get_file_index(filename);
  if (file_location == -1)
    return -1;

  char temp_ptr[6];
  int temp_ptr2;

  // move to correct data location and write new data over old data
  fseek(virtual_disk, (file_location - 1) * 16, SEEK_SET);
	fseek(virtual_disk, 19, SEEK_CUR);
	fread(temp_ptr , 1, 6, virtual_disk);
	temp_ptr2 = atoi(temp_ptr);
	fseek(virtual_disk, (temp_ptr2 - 1) * 16, SEEK_SET);
	fwrite(new_data, 1, DISK_BLOCK_SIZE, virtual_disk);
  
  return 1;
}

void close_virtual_disk_properly() {
  fclose(virtual_disk);
  exit(EXIT_SUCCESS);
}

int get_file_index(char *filename) {
  char temp_fn[13];
  int temp_index = 0;

  // same as below functions, loop until name (hopefully) matches
  fseek(virtual_disk, 0, SEEK_SET);
  int temp = DISK_BLOCK_SIZE * RESERVED_SP;
  for (int index=0; index<temp; index+=FAT_SIZE) {
    fseek(virtual_disk, index + 1, SEEK_SET);
    fread(temp_fn, 13, 1, virtual_disk);
    if (strcmp(temp_fn, filename) == 0) {
      temp_index = index / 16 + 1;
      return temp_index;
    }
  }
  
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
      sprintf(temp_ptr, "%u", index / 16 + 1);
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
      int location = ((index / ENTRY_SIZE * 4) + (RESERVED_SP * DISK_BLOCK_SIZE / 16) + 1);
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
      int location = ((index / DISK_BLOCK_SIZE * ENTRY_SIZE) + (DATA_SP * DISK_BLOCK_SIZE / 16) + 1);
      sprintf(temp_ptr, "%u", location);
      return temp_ptr;
    }
  }

  // if no open data blocks then return -1
  return "-1";
}
