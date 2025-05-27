#include "arch/x86/gdt.h"
#include "arch/x86/interrupt.h"
#include "arch/x86/tss.h"
#include "arch/x86/syscall.h"
#include "drivers/serial.h"
#include "lib/log.h"
#include "lib/string.h"
#include "mem/page_frame_allocator.h"
#include "mem/paging.h"
#include "mem/process.h"
#include "multiboot.h"
#include "fs/vfs.h"
#include "fs/initrd.h"

void test_read_file(const char* path) {
  LOG_INFO("Testing read for file: %s", path);

  fs_descriptor_t fd;
  if (open_file(path, &fd) != 0) {
    LOG_ERROR("Failed to open file: %s", path);
    return;
  }

  LOG_INFO("Opened file %s (size: %d bytes)", path, fd.size);

  char buffer[512];
  if (fd.size >= sizeof(buffer)) {
    LOG_ERROR("File too large for buffer");
    close_file(&fd);
    return;
  }

  int bytes_read = read_file(&fd, buffer, fd.size);
  if (bytes_read < 0) {
    LOG_ERROR("Error reading file");
    close_file(&fd);
    return;
  }

  buffer[bytes_read] = '\0';

  LOG_INFO("Read %d bytes from file", bytes_read);
  LOG_INFO("File contents: %s", buffer);

  close_file(&fd);
}

void test_list_directory(void) {
  LOG_INFO("Listing root directory contents:");

  fs_descriptor_t fd;
  if (open_file("/", &fd) != 0) {
    LOG_ERROR("Failed to open root directory");
    return;
  }

  char name[64];
  while (read_directory(&fd, name, sizeof(name))) {
    LOG_INFO("  %s", name);
  }

  close_file(&fd);

  LOG_LINE();
}

int kmain(multiboot_info_t* mbinfo, uint32_t magic_number, kernel_meminfo_t mem) {
  log_init();
  LOG_INFO("Kernel starting...\n");

  if (magic_number != MULTIBOOT_BOOTLOADER_MAGIC) {
    LOG_ERROR("Invalid magic number: 0x%x", magic_number);
    while (1)
      __asm__("hlt");
    return 1;
  }

  gdt_init();
  tss_init();
  interrupt_init();
  syscall_init();
  init_page_frame_allocator(mem.kernel_physical_start, mem.kernel_physical_end, mem.kernel_virtual_start,
                            mem.kernel_virtual_end);
  init_process_manager();

  init_vfs();

  if (mbinfo->mods_count > 0) {
    LOG_INFO("Modules found: %d", mbinfo->mods_count);

    if (mbinfo->flags & MULTIBOOT_INFO_MODS) {
      uint32_t mods_addr_phys = mbinfo->mods_addr;
      uint32_t mods_addr_virt = phys_to_virt(mods_addr_phys);
      multiboot_module_t* modules = (multiboot_module_t*)mods_addr_virt;

      for (uint32_t i = 0; i < mbinfo->mods_count; i++) {
        multiboot_module_t* module = &modules[i];
        uint32_t module_start_phys = module->mod_start;
        uint32_t module_end_phys = module->mod_end;
        uint32_t module_size = module_end_phys - module_start_phys;
        uint32_t module_start_virt = phys_to_virt(module_start_phys);

        LOG_INFO("Module %d physical address: 0x%x - 0x%x (size: %d bytes)",
                i, module_start_phys, module_end_phys, module_size);
        LOG_INFO("Module %d virtual address: 0x%x", i, module_start_virt);

        if (module->cmdline) {
          char* module_cmdline = (char*)phys_to_virt(module->cmdline);
          LOG_INFO("Module %d cmdline: %s", i, module_cmdline);

          if (i > 0) {
            LOG_INFO("Found initrd module at index %d", i);

            init_initrd((void*)module_start_virt);

            if (mount_fs("/", "initrd", get_initrd_ops(), NULL) == 0) {
              LOG_INFO("Successfully mounted initrd filesystem at /");

              test_list_directory();
              test_read_file("/hello.txt");
              test_read_file("/test.txt");
              test_read_file("/README.txt");
            } else {
              LOG_ERROR("Failed to mount initrd filesystem");
            }
          }
        }

        if (i == 0) {
          process_t* module_proc = create_process((void*)module_start_virt, module_size);
          if (module_proc == NULL) {
            LOG_ERROR("Failed to create process for module");
          } else {
            LOG_INFO("Created user process with PID: %d to run module", module_proc->pid);
          }
        }
      }
    } else {
      LOG_ERROR("Module information not available in multiboot structure");
    }
  } else {
    LOG_INFO("No modules found to load");
  }

  LOG_DEBUG("mbinfo: 0x%x", mbinfo);
  LOG_DEBUG("magic number: 0x%x", magic_number);
  interrupt_sti();
  LOG_INFO("Kernel initialized successfully\n");

  schedule();

  LOG_INFO("No process to schedule, halting...");

  while (1) {
    __asm__("hlt");
  }

  return 0;
}
