#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>

#define INITRD_MAGIC 0xBF
#define MAX_FILES 64
#define MAX_PATH 256

typedef enum {
    FS_TYPE_FILE = 0,
    FS_TYPE_DIRECTORY = 1,
    FS_TYPE_CHAR_DEVICE = 2,
    FS_TYPE_BLOCK_DEVICE = 3,
    FS_TYPE_PIPE = 4,
    FS_TYPE_SYMLINK = 5,
    FS_TYPE_MOUNT = 6
} fs_type_t;

typedef enum {
    FS_PERM_READ = 0x1,
    FS_PERM_WRITE = 0x2,
    FS_PERM_EXECUTE = 0x4
} fs_perm_t;

typedef struct {
    char name[32];
    uint32_t size;
    uint32_t offset;
    fs_type_t type;
    uint32_t permissions;
} initrd_file_header_t;

typedef struct {
    uint32_t magic;
    uint32_t num_files;
    initrd_file_header_t files[MAX_FILES];
} initrd_header_t;

void add_file(initrd_header_t* header, FILE* output, const char* filename, const char* filepath) {
    FILE* input = fopen(filepath, "rb");
    if (!input) {
        fprintf(stderr, "Error: Cannot open file %s\n", filepath);
        return;
    }

    fseek(input, 0, SEEK_END);
    uint32_t file_size = ftell(input);
    fseek(input, 0, SEEK_SET);

    strncpy(header->files[header->num_files].name, filename, 31);
    header->files[header->num_files].name[31] = '\0';

    header->files[header->num_files].size = file_size;
    header->files[header->num_files].type = FS_TYPE_FILE;
    header->files[header->num_files].permissions = FS_PERM_READ;

    uint32_t offset = 0;
    for (uint32_t i = 0; i < header->num_files; i++) {
        offset += header->files[i].size;
    }
    header->files[header->num_files].offset = offset;

    long file_pos = ftell(output);

    header->num_files++;

    fseek(output, 0, SEEK_END);

    uint8_t buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        fwrite(buffer, 1, bytes_read, output);
    }

    fclose(input);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <output file> <file1> [file2] ...\n", argv[0]);
        return 1;
    }

    FILE* output = fopen(argv[1], "wb");
    if (!output) {
        fprintf(stderr, "Error: Cannot create output file %s\n", argv[1]);
        return 1;
    }

    initrd_header_t header;
    memset(&header, 0, sizeof(header));
    header.magic = INITRD_MAGIC;
    header.num_files = 0;

    char signature[8] = "INITRD\0\0";
    fwrite(signature, 1, 8, output);

    fwrite(&header, sizeof(header), 1, output);

    long data_pos = ftell(output);

    for (int i = 2; i < argc; i++) {
        char* filepath = argv[i];
        char* filename = strrchr(filepath, '/');
        if (filename) {
            filename++;
        } else {
            filename = filepath;
        }

        printf("Adding file %s as %s\n", filepath, filename);
        add_file(&header, output, filename, filepath);
    }

    fseek(output, 8, SEEK_SET);
    fwrite(&header, sizeof(header), 1, output);

    fclose(output);
    printf("Created initrd with %d files\n", header.num_files);

    return 0;
}
