#ifndef INITRD_H
#define INITRD_H

#include "fs/vfs.h"
#include <stdint.h>

#define INITRD_MAX_NAME_LEN 32

#define INITRD_MAX_FILES 64

#define INITRD_MAGIC 0xBF

typedef struct {
  char name[INITRD_MAX_NAME_LEN];
  uint32_t size;
  uint32_t offset;
  fs_type_t type;
  uint32_t permissions;
} initrd_file_header_t;

typedef struct {
  uint32_t magic;
  uint32_t num_files;
  initrd_file_header_t files[INITRD_MAX_FILES];
} initrd_header_t;

void init_initrd(void* initrd_location);

fs_ops_t* get_initrd_ops(void);

#endif /* INITRD_H */
