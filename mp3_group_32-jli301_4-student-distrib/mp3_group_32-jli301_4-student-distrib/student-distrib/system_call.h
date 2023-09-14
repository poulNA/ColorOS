#ifndef _SYS_CALL_H
#define _SYS_CALL_H

#ifndef ASM

#include "lib.h"
#include "file_system.h"
#include "file_system_driver.h"
#include "rtc.h"
#include "terminal_driver.h"
#include "interrupt_linkage.h"
#include "paging.h"
#include "x86_desc.h"

#define MAX_FILES 8 // max number of files in file descriptor array
#define addr_8MB 0x800000 // hex value for 8MB addr
#define _4MB    0x400000 // hex value for 4 MB value
#define size_8kb 0x2000 // hex value for 8 kB value

#define USER_PAGE_DIR_IDX 32 // location in page directory for user space
#define USER_SPACE 0x8000000 // hex value for address of user space
#define PROGRAM_IMG 0x08048000 // hex value for address of program image
#define VID_MEM_DIR 0x8400000 // location of page directory where virtual video mem is located
#define VID_MEM_ADDR 0x84b8000 // virtual location of video mem
#define OVER_MAX_PROCESSES 2 // Max number of process that can be run, will be 6 for checkpoint 5

// struct for a file_operation table
typedef struct file_operations {
    int32_t (*open)(const uint8_t* file_name);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} file_operations_t;

// struct for a file descriptor table
typedef struct fd {
    file_operations_t* file_ops_table_ptr;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags;
} fd_t;

// struct for a pcb block
typedef struct pcb {
    uint8_t pid;
    uint8_t parent_pid;
    fd_t file_descriptor[MAX_FILES];
    uint8_t file[MAX_NAME_LENGTH];
    uint8_t arg[KEY_BUFF_SIZE];
    uint8_t file_len;
    uint8_t arg_len;
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint8_t active;
} pcb_t;

int32_t sys_halt (uint8_t status);
int32_t sys_execute (const uint8_t* command);
int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t sys_open (const uint8_t* filename);
int32_t sys_close (int32_t fd);
int32_t sys_getargs (uint8_t* buf, int32_t nbytes);
int32_t sys_vidmap (uint8_t** screen_start);
int32_t sys_set_handler (int32_t signum, void* handler_address);
int32_t sys_sigreturn (void);

void file_desc_init();
int32_t bad_call();
int get_PCB_addr();
pcb_t get_cur_PCB();
int find_next_fd_index(pcb_t p);
void map(void* vaddr, void* paddr);
void flush_TLB();

extern void iret_setup(uint32_t eip);
extern void ret_halt(uint32_t eax, uint32_t ebp, uint32_t esp);
extern void sys_call_handler();

#endif
#endif
