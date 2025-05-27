#include "fs/vfs.h"
#include "lib/log.h"
#include "lib/string.h"
#include <stddef.h>

#define MAX_MOUNT_POINTS 8
#define MAX_PATH_LEN 256

static mount_point_t mount_points[MAX_MOUNT_POINTS];
static int num_mount_points = 0;
static char path_buffers[MAX_MOUNT_POINTS][MAX_PATH_LEN];
static char device_buffers[MAX_MOUNT_POINTS][MAX_PATH_LEN];

void init_vfs(void) {
    num_mount_points = 0;
    memset(mount_points, 0, sizeof(mount_points));
    LOG_INFO("VFS initialized");
}

int mount_fs(const char* path, const char* device, fs_ops_t* ops, void* private_data) {
    if (num_mount_points >= MAX_MOUNT_POINTS) {
        LOG_ERROR("Maximum number of mount points reached");
        return -1;
    }

    for (int i = 0; i < num_mount_points; i++) {
        if (strcmp(mount_points[i].path, path) == 0) {
            LOG_ERROR("Path %s is already mounted", path);
            return -1;
        }
    }

    int idx = num_mount_points;

    strncpy(path_buffers[idx], path, MAX_PATH_LEN - 1);
    path_buffers[idx][MAX_PATH_LEN - 1] = '\0';

    strncpy(device_buffers[idx], device, MAX_PATH_LEN - 1);
    device_buffers[idx][MAX_PATH_LEN - 1] = '\0';

    mount_points[idx].path = path_buffers[idx];
    mount_points[idx].device = device_buffers[idx];
    mount_points[idx].ops = ops;
    mount_points[idx].private_data = private_data;
    num_mount_points++;

    LOG_INFO("Mounted %s at %s", device, path);
    return 0;
}

int unmount_fs(const char* path) {
    for (int i = 0; i < num_mount_points; i++) {
        if (strcmp(mount_points[i].path, path) == 0) {
            for (int j = i; j < num_mount_points - 1; j++) {
                mount_points[j] = mount_points[j + 1];

                strcpy(path_buffers[j], path_buffers[j + 1]);
                strcpy(device_buffers[j], device_buffers[j + 1]);
            }
            num_mount_points--;

            LOG_INFO("Unmounted %s", path);
            return 0;
        }
    }

    LOG_ERROR("Mount point %s not found", path);
    return -1;
}

static mount_point_t* find_mount_point(const char* path) {
    mount_point_t* best_match = NULL;
    size_t best_match_len = 0;

    for (int i = 0; i < num_mount_points; i++) {
        size_t mount_path_len = strlen(mount_points[i].path);

        if (mount_path_len == 1 && mount_points[i].path[0] == '/' && path[0] == '/') {
            if (best_match_len == 0) {
                best_match = &mount_points[i];
                best_match_len = 1;
            }
            continue;
        }

        if (strncmp(path, mount_points[i].path, mount_path_len) == 0) {
            if (path[mount_path_len] == '/' || path[mount_path_len] == '\0') {
                if (mount_path_len > best_match_len) {
                    best_match = &mount_points[i];
                    best_match_len = mount_path_len;
                }
            }
        }
    }

    LOG_DEBUG("Found mount point %s for path %s",
             best_match ? best_match->path : "NONE", path);

    return best_match;
}

static mount_point_t* find_mount_point_by_inode(uint32_t inode) {
    (void)inode;
    if (num_mount_points > 0) {
        return &mount_points[0];
    }
    return NULL;
}

int open_file(const char* path, fs_descriptor_t* fd) {
    mount_point_t* mount = find_mount_point(path);
    if (!mount) {
        LOG_ERROR("No mount point found for %s", path);
        return -1;
    }

    if (!mount->ops->open) {
        LOG_ERROR("Filesystem does not support open operation");
        return -1;
    }

    return mount->ops->open(path, fd);
}

int close_file(fs_descriptor_t* fd) {
    mount_point_t* mount = find_mount_point_by_inode(fd->inode);
    if (!mount) {
        LOG_ERROR("No mount point found for file descriptor");
        return -1;
    }

    if (!mount->ops->close) {
        LOG_ERROR("Filesystem does not support close operation");
        return -1;
    }

    return mount->ops->close(fd);
}

int read_file(fs_descriptor_t* fd, void* buffer, uint32_t size) {
    mount_point_t* mount = find_mount_point_by_inode(fd->inode);
    if (!mount) {
        LOG_ERROR("No mount point found for file descriptor");
        return -1;
    }

    if (!mount->ops->read) {
        LOG_ERROR("Filesystem does not support read operation");
        return -1;
    }

    return mount->ops->read(fd, buffer, size);
}

int write_file(fs_descriptor_t* fd, const void* buffer, uint32_t size) {
    mount_point_t* mount = find_mount_point_by_inode(fd->inode);
    if (!mount) {
        LOG_ERROR("No mount point found for file descriptor");
        return -1;
    }

    if (!mount->ops->write) {
        LOG_ERROR("Filesystem does not support write operation");
        return -1;
    }

    return mount->ops->write(fd, buffer, size);
}

int seek_file(fs_descriptor_t* fd, uint32_t offset) {
    mount_point_t* mount = find_mount_point_by_inode(fd->inode);
    if (!mount) {
        LOG_ERROR("No mount point found for file descriptor");
        return -1;
    }

    if (!mount->ops->seek) {
        LOG_ERROR("Filesystem does not support seek operation");
        return -1;
    }

    return mount->ops->seek(fd, offset);
}

int read_directory(fs_descriptor_t* fd, char* name, uint32_t size) {
    mount_point_t* mount = find_mount_point_by_inode(fd->inode);
    if (!mount) {
        LOG_ERROR("No mount point found for file descriptor");
        return -1;
    }

    if (!mount->ops->readdir) {
        LOG_ERROR("Filesystem does not support readdir operation");
        return -1;
    }

    return mount->ops->readdir(fd, name, size);
}

int create_directory(const char* path) {
    mount_point_t* mount = find_mount_point(path);
    if (!mount) {
        LOG_ERROR("No mount point found for %s", path);
        return -1;
    }

    if (!mount->ops->mkdir) {
        LOG_ERROR("Filesystem does not support mkdir operation");
        return -1;
    }

    return mount->ops->mkdir(path);
}

int remove_directory(const char* path) {
    mount_point_t* mount = find_mount_point(path);
    if (!mount) {
        LOG_ERROR("No mount point found for %s", path);
        return -1;
    }

    if (!mount->ops->rmdir) {
        LOG_ERROR("Filesystem does not support rmdir operation");
        return -1;
    }

    return mount->ops->rmdir(path);
}

int create_file(const char* path) {
    mount_point_t* mount = find_mount_point(path);
    if (!mount) {
        LOG_ERROR("No mount point found for %s", path);
        return -1;
    }

    if (!mount->ops->create) {
        LOG_ERROR("Filesystem does not support create operation");
        return -1;
    }

    return mount->ops->create(path);
}

int delete_file(const char* path) {
    mount_point_t* mount = find_mount_point(path);
    if (!mount) {
        LOG_ERROR("No mount point found for %s", path);
        return -1;
    }

    if (!mount->ops->unlink) {
        LOG_ERROR("Filesystem does not support unlink operation");
        return -1;
    }

    return mount->ops->unlink(path);
}
