#ifndef VFS_H
#define VFS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  FS_TYPE_FILE,
  FS_TYPE_DIRECTORY,
  FS_TYPE_CHAR_DEVICE,
  FS_TYPE_BLOCK_DEVICE,
  FS_TYPE_PIPE,
  FS_TYPE_SYMLINK,
  FS_TYPE_MOUNT
} fs_type_t;

typedef enum { FS_PERM_READ = 0x1, FS_PERM_WRITE = 0x2, FS_PERM_EXECUTE = 0x4 } fs_perm_t;

typedef struct {
  uint32_t inode;
  uint32_t position;
  fs_type_t type;
  uint32_t size;
  uint32_t permissions;
} fs_descriptor_t;

typedef struct {
  int (*open)(const char* path, fs_descriptor_t* fd);
  int (*close)(fs_descriptor_t* fd);
  int (*read)(fs_descriptor_t* fd, void* buffer, uint32_t size);
  int (*write)(fs_descriptor_t* fd, const void* buffer, uint32_t size);
  int (*seek)(fs_descriptor_t* fd, uint32_t offset);

  int (*readdir)(fs_descriptor_t* fd, char* name, uint32_t size);
  int (*mkdir)(const char* path);
  int (*rmdir)(const char* path);

  int (*create)(const char* path);
  int (*unlink)(const char* path);

  int (*mount)(const char* path, const char* device);
  int (*unmount)(const char* path);
} fs_ops_t;

typedef struct {
  char* path;
  char* device;
  fs_ops_t* ops;
  void* private_data;
} mount_point_t;

void init_vfs(void);
int mount_fs(const char* path, const char* device, fs_ops_t* ops, void* private_data);
int unmount_fs(const char* path);
int open_file(const char* path, fs_descriptor_t* fd);
int close_file(fs_descriptor_t* fd);
int read_file(fs_descriptor_t* fd, void* buffer, uint32_t size);
int write_file(fs_descriptor_t* fd, const void* buffer, uint32_t size);
int seek_file(fs_descriptor_t* fd, uint32_t offset);
int read_directory(fs_descriptor_t* fd, char* name, uint32_t size);
int create_directory(const char* path);
int remove_directory(const char* path);
int create_file(const char* path);
int delete_file(const char* path);

#endif /* VFS_H */
