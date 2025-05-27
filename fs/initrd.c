#include "fs/initrd.h"
#include "fs/vfs.h"
#include "lib/log.h"
#include "lib/string.h"
#include <stddef.h>

#define INITRD_MAGIC 0xBF
#define INITRD_SIGNATURE "INITRD"

static initrd_header_t* initrd_header = NULL;
static uint8_t* initrd_data = NULL;

void init_initrd(void* initrd_location) {
    uint8_t* data = (uint8_t*)initrd_location;
    LOG_DEBUG("Module data first 32 bytes:");
    for (int i = 0; i < 4; i++) {
        LOG_DEBUG("  %02x %02x %02x %02x %02x %02x %02x %02x",
                 data[i*8+0], data[i*8+1], data[i*8+2], data[i*8+3],
                 data[i*8+4], data[i*8+5], data[i*8+6], data[i*8+7]);
    }

    if (strncmp((char*)data, INITRD_SIGNATURE, 6) == 0) {
        LOG_INFO("INITRD signature found at the beginning of module");
        initrd_header = (initrd_header_t*)(data + 8);
    } else {
        for (int i = 0; i < 64; i++) {
            if (*(uint32_t*)(data + i) == INITRD_MAGIC) {
                LOG_INFO("Found initrd magic at offset %d", i);
                initrd_header = (initrd_header_t*)(data + i);
                break;
            }
        }
    }

    if (!initrd_header) {
        LOG_WARN("No valid initrd found, creating a temporary one for testing");

        static initrd_header_t temp_header;
        static initrd_file_header_t temp_files[1];
        static char temp_data[] = "This is a test file created in memory.";

        temp_header.magic = INITRD_MAGIC;
        temp_header.num_files = 1;

        strcpy(temp_files[0].name, "test.txt");
        temp_files[0].size = sizeof(temp_data) - 1;
        temp_files[0].offset = 0;
        temp_files[0].type = FS_TYPE_FILE;
        temp_files[0].permissions = FS_PERM_READ;

        initrd_header = &temp_header;
        memcpy(&temp_header.files[0], &temp_files[0], sizeof(initrd_file_header_t));
        initrd_data = (uint8_t*)temp_data;

        LOG_INFO("Created temporary initrd with %d files", initrd_header->num_files);
        return;
    }

    LOG_DEBUG("Initrd header at 0x%x", (uint32_t)initrd_header);
    LOG_DEBUG("Initrd magic found: 0x%08x (expected: 0x%08x)", initrd_header->magic, INITRD_MAGIC);

    if (initrd_header->magic != INITRD_MAGIC) {
        LOG_ERROR("Invalid initrd magic number");
        return;
    }

    initrd_data = (uint8_t*)initrd_header + sizeof(initrd_header_t);

    LOG_INFO("Initialized initrd with %d files", initrd_header->num_files);

    for (uint32_t i = 0; i < initrd_header->num_files; i++) {
        LOG_DEBUG("File %d: %s (size: %d bytes, offset: 0x%x)",
                 i, initrd_header->files[i].name, initrd_header->files[i].size,
                 initrd_header->files[i].offset);
    }
}

static initrd_file_header_t* find_file(const char* path) {
    if (!initrd_header) {
        return NULL;
    }

    if (path[0] == '/') {
        path++;
    }

    for (uint32_t i = 0; i < initrd_header->num_files; i++) {
        if (strcmp(initrd_header->files[i].name, path) == 0) {
            return &initrd_header->files[i];
        }
    }

    return NULL;
}

static int initrd_open(const char* path, fs_descriptor_t* fd) {
    if (strcmp(path, "/") == 0) {
        LOG_DEBUG("Opening root directory");
        fd->inode = 0xFFFFFFFF;
        fd->position = 0;
        fd->type = FS_TYPE_DIRECTORY;
        fd->size = 0;
        fd->permissions = FS_PERM_READ;
        return 0;
    }

    initrd_file_header_t* file = find_file(path);
    if (!file) {
        LOG_ERROR("File not found: %s", path);
        return -1;
    }

    fd->inode = (uint32_t)(file - initrd_header->files);
    fd->position = 0;
    fd->type = file->type;
    fd->size = file->size;
    fd->permissions = file->permissions;

    LOG_DEBUG("Opened file %s (size: %d bytes)", path, file->size);
    return 0;
}

static int initrd_close(fs_descriptor_t* fd) {
    (void)fd;
    return 0;
}

static int initrd_read(fs_descriptor_t* fd, void* buffer, uint32_t size) {
    if (fd->inode >= initrd_header->num_files) {
        LOG_ERROR("Invalid inode number: %d", fd->inode);
        return -1;
    }

    initrd_file_header_t* file = &initrd_header->files[fd->inode];

    if (fd->position >= file->size) {
        return 0;
    }

    uint32_t remaining = file->size - fd->position;
    uint32_t to_read = (size < remaining) ? size : remaining;

    memcpy(buffer, initrd_data + file->offset + fd->position, to_read);
    fd->position += to_read;

    return to_read;
}

static int initrd_write(fs_descriptor_t* fd, const void* buffer, uint32_t size) {
    (void)fd;
    (void)buffer;
    (void)size;
    LOG_ERROR("Write not supported in initrd");
    return -1;
}

static int initrd_seek(fs_descriptor_t* fd, uint32_t offset) {
    if (fd->inode >= initrd_header->num_files) {
        LOG_ERROR("Invalid inode number: %d", fd->inode);
        return -1;
    }

    initrd_file_header_t* file = &initrd_header->files[fd->inode];

    if (offset > file->size) {
        LOG_ERROR("Seek beyond end of file");
        return -1;
    }

    fd->position = offset;
    return 0;
}

static int initrd_readdir(fs_descriptor_t* fd, char* name, uint32_t size) {
    static uint32_t current_file = 0;

    if (fd->type == FS_TYPE_DIRECTORY) {
        if (current_file >= initrd_header->num_files) {
            current_file = 0;
            return 0;
        }

        strncpy(name, initrd_header->files[current_file].name, size - 1);
        name[size - 1] = '\0';

        current_file++;
        return 1;
    } else {
        LOG_ERROR("Not a directory");
        return 0;
    }
}

static int initrd_mkdir(const char* path) {
    (void)path;
    LOG_ERROR("Mkdir not supported in initrd");
    return -1;
}

static int initrd_rmdir(const char* path) {
    (void)path;
    LOG_ERROR("Rmdir not supported in initrd");
    return -1;
}

static int initrd_create(const char* path) {
    (void)path;
    LOG_ERROR("Create not supported in initrd");
    return -1;
}

static int initrd_unlink(const char* path) {
    (void)path;
    LOG_ERROR("Unlink not supported in initrd");
    return -1;
}

static int initrd_mount(const char* path, const char* device) {
    (void)path;
    (void)device;
    LOG_ERROR("Mount not supported in initrd");
    return -1;
}

static int initrd_unmount(const char* path) {
    (void)path;
    LOG_ERROR("Unmount not supported in initrd");
    return -1;
}

fs_ops_t* get_initrd_ops(void) {
    static fs_ops_t ops = {
        .open = initrd_open,
        .close = initrd_close,
        .read = initrd_read,
        .write = initrd_write,
        .seek = initrd_seek,
        .readdir = initrd_readdir,
        .mkdir = initrd_mkdir,
        .rmdir = initrd_rmdir,
        .create = initrd_create,
        .unlink = initrd_unlink,
        .mount = initrd_mount,
        .unmount = initrd_unmount
    };

    return &ops;
}
