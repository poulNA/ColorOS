#include "system_call.h"

pcb_t cur_pcb; // global variables to keep track of pcb and pid
int cur_pid;
int num_processes;

uint8_t magic_num[4] = {0x7f, 0x45, 0x4c, 0x46}; // array with magic numbers to check in meta date to see if an EXE file

// file operations tables for files, directories, terminal, rtc, stdin, and stdout
struct file_operations reg_file = {
    .open = &file_open,
    .read = &file_read,
    .write = &file_write,
    .close = &file_close};

struct file_operations reg_dir = {
    .open = &directory_open,
    .read = &directory_read,
    .write = &directory_write,
    .close = &directory_close};

struct file_operations reg_terminal = {
    .open = &terminal_open,
    .read = &terminal_read,
    .write = &terminal_write,
    .close = &terminal_close};

struct file_operations reg_rtc = {
    .open = &rtc_open,
    .read = &rtc_read,
    .write = &rtc_write,
    .close = &rtc_close};

struct file_operations reg_stdin = {
    .open = &bad_call,
    .read = &terminal_read,
    .write = &terminal_write,
    .close = &bad_call};

struct file_operations reg_stdout = {
    .open = &bad_call,
    .read = &terminal_read,
    .write = &terminal_write,
    .close = &bad_call};

/* void file_desc_init()
 * Inputs: none
 * Return Value: none
 * Function: initializes a file descriptor */
void file_desc_init()
{
    cur_pcb = *(pcb_t *)(get_PCB_addr()); // gets current pcb

    // Mark stdin and stdout as "Open"
    cur_pcb.file_descriptor[0].flags = 1;
    cur_pcb.file_descriptor[0].file_ops_table_ptr = &reg_stdin; // or without .read
    cur_pcb.file_descriptor[1].flags = 1;
    cur_pcb.file_descriptor[1].file_ops_table_ptr = &reg_stdout;

    // initializes rest of entries in file descriptor array to empty
    int i;
    for (i = 2; i < MAX_FILES; i++)
    {
        cur_pcb.file_descriptor[i].file_ops_table_ptr = NULL;
        cur_pcb.file_descriptor[i].inode = -1;
        cur_pcb.file_descriptor[i].file_position = 0;
        cur_pcb.file_descriptor[i].flags = 0;
    }
}

/* int32_t bad_call()
 * Inputs: none
 * Return Value: -1
 * Function: returns -1 for a bad call */
int32_t bad_call()
{
    return -1;
}

/* int32_t sys_halt (uint8_t status)
 *  input   : status of system
 *  output  : nothing
 *  return  : 1 for succcess, -1 for failure
 *  Description : halts the process of system
 */
int32_t sys_halt(uint8_t status)
{
    /* Set up Return Value */
    // Check if exception
    if (status == (uint8_t)50)
    {
        status = (uint8_t)256;
    } // anywhere 0-255 unused
    // Check if program finished

    /* Close all open files */
    pcb_t *pcb_to_clear = (pcb_t *)get_PCB_addr(); // Get the address of the PCB to clear

    int fd_index;
    for (fd_index = 0; fd_index < MAX_FILES; fd_index++)
    {                                                                      // Loop to clear all fds
        pcb_to_clear->file_descriptor[fd_index].file_ops_table_ptr = NULL; // Set file ops to NULL, and set all flags to 0
        pcb_to_clear->file_descriptor[fd_index].inode = -1;
        pcb_to_clear->file_descriptor[fd_index].file_position = 0;
        pcb_to_clear->file_descriptor[fd_index].flags = 0;
    }

    memset(pcb_to_clear->file, 0, sizeof(pcb_to_clear->file)); // clearing added elements of pcb
    memset(pcb_to_clear->arg, 0, sizeof(pcb_to_clear->arg));
    pcb_to_clear->file_len = 0;
    pcb_to_clear->arg_len = 0;

    if (cur_pid <= 0)
    { // If this is the last process, restart the main shell.
        num_processes--;
        sys_execute((const uint8_t *)"shell");
        return -1;
    }

    cur_pid = pcb_to_clear->parent_pid; // cur_pid = parent's, pcb_to_clear = current pcb to be halted
    cur_pcb = *(pcb_t *)(get_PCB_addr());
    pcb_to_clear->active = 0; // Set process to inactive (As per review slides)

    num_processes--; // Decrement the number of processes

    /* Check if main shell */

    /* Not main shell handler */
    // Get Parent process

    // Set TSS for parent
    tss.ss0 = KERNEL_DS;                            // sets the ss0 in TSS to be the Kernal for memory
    tss.esp0 = addr_8MB - (size_8kb * cur_pid) - 4; // kernel stack pointer

    // Map parent's paging
    map((void *)USER_SPACE, (void *)(addr_8MB + cur_pid * _4MB)); // uses the map function to map the parent page

    flush_TLB(); // resets the CR3 value

    // Set parent's process as active
    cur_pcb.active = 1;

    /* Halt return */
    // ret_halt(status, cur_pcb.saved_ebp, cur_pcb.saved_esp);
    asm volatile( // This asm will set the ebp, esp and call the execute return to return back to the handler
        ""
        "movl %0, %%eax \n\t"
        "movl %1, %%ebp \n\t"
        "movl %2, %%esp \n\t"
        "leave \n\t"
        "ret"
        :
        : "r"((uint32_t)status), "r"(pcb_to_clear->saved_ebp), "r"(pcb_to_clear->saved_esp)
        : "%eax");

    return 1;
}

/* int32_t sys_execute (const uint8_t* command)
 *  input   : pointer to command buffer
 *  output  : nothing
 *  return  : 0 if sucessful, -1 if fail
 *  Description : This function intakes the command buffer, checks if it is an EXE file, and executes the specific file function.
 */
int32_t sys_execute(const uint8_t *command)
{
    int i, j;
    int argFlag = 0;
    uint32_t cmd_addr;

    if (num_processes >= OVER_MAX_PROCESSES)
    { // Make sure we do not go above the maximum number of processes
        return -1;
    }

    pcb_t *new_pcb_ptr = (pcb_t *)(addr_8MB - (size_8kb * (num_processes + 1))); // Different from PID

    /* Parse cmd */
    // parse the command and grab the command and argumends seperate
    // file name seperate than arguments, will set up init function for execute

    if (command == NULL)
    {
        return -1;
    } // checks if the command is valid (ie. NULL or just Enter)
    if (command[0] == ENTER || command[0] == '\0')
    {
        return -1;
    }

    for (i = 0; i < MAX_NAME_LENGTH; i++)
    { // cycles through the command and saves the file into the command file buffer. Ends with space, enter, or null
        if (command[i] == ' ' || command[i] == ENTER || command[i] == '\0')
        {
            break;
        }

        new_pcb_ptr->file[i] = command[i]; // adds command file to file buffer and records length (needed for argument buffer)
        new_pcb_ptr->file_len++;
    }

    if (command[i] == ' ')
    { // checks if there is arguments to fill cmd_arg
        argFlag = 1;
        i += 1;
    }

    for (j = 0; j < (KEY_BUFF_SIZE + 1) && argFlag == 1; j++)
    { // our keyboard buffer is 127 + preset enter in 128
        if (command[i] == ENTER || command[i] == '\0')
        { // cycles through the command and saves the arguments into the command argument buffer
            break;
        }

        new_pcb_ptr->arg[j] = command[i];
        i++;
        new_pcb_ptr->arg_len++;
    }

    dentry_t cmd_dentry;

    /* File Checks */
    // check if valid file -> call the dentry open stuff
    // uint8_t
    if (read_dentry_by_name(new_pcb_ptr->file, &(cmd_dentry)) == -1)
    { // invalid file
        memset(new_pcb_ptr->file, 0, sizeof(new_pcb_ptr->file));
        memset(new_pcb_ptr->arg, 0, sizeof(new_pcb_ptr->file));
        new_pcb_ptr->file_len = 0;
        new_pcb_ptr->arg_len = 0;
        return -1;
    }

    uint8_t magic_num_buf[4]; // instantiates the buffers for the two read datas
    uint8_t eip_buf[4];

    int32_t cmd_read_1 = 0;
    int32_t cmd_read_2 = 0;

    cmd_read_1 = read_data(cmd_dentry.inode_number, 0, magic_num_buf, 4); // read first 4 bytes of inode at address
    if (cmd_read_1 != 4)
    {
        return -1;
    } // did not read all 4.. return -1?

    int exeFlag = 0;
    int count = 0;
    for (i = 0; i < 4; i++)
    { // if the magic number command buffer doesnt match the magic numbers, its not an exe file
        if (i == 3 && count == 3)
        {
            if (magic_num_buf[i] == magic_num[i])
            {
                exeFlag = 1;
            }
        }
        if (magic_num_buf[i] == magic_num[i])
        {
            count++;
        }
    }

    if (exeFlag != 1)
    {
        return -1;
    } // if not an EXE file, then return FAIL

    cmd_read_2 = read_data(cmd_dentry.inode_number, 24, eip_buf, 4); // read first 4 bytes of inode at address
    if (cmd_read_2 != 4)
    {
        return -1;
    } // did not read all 4.. return -1?

    cmd_addr = (eip_buf[3] << 24 | eip_buf[2] << 16 | eip_buf[1] << 8 | eip_buf[0]); // its loaded in backwards, bit shifts the buffer bytes into correct 32bit value.

    if (num_processes == 0)
    {
        new_pcb_ptr->parent_pid = -1; // If this is the first process, set parent PID to -1
    }
    else
    {
        new_pcb_ptr->parent_pid = cur_pid; // Otherwise, set the parent PID to the PID of the previous PCB
    }
    new_pcb_ptr->pid = num_processes; // Set the new PCB's PID, set the new PCB as active, and deactivate the old PCB
    cur_pcb.active = 0;               // Only changed these because the review slides said to
    new_pcb_ptr->active = 1;

    new_pcb_ptr->file_descriptor[0].flags = 1; // Set in-use flags to 1 and add stdin and stdout as operations
    new_pcb_ptr->file_descriptor[0].file_ops_table_ptr = &reg_stdin;
    new_pcb_ptr->file_descriptor[1].flags = 1;
    new_pcb_ptr->file_descriptor[1].file_ops_table_ptr = &reg_stdout;

    for (j = 2; j < MAX_FILES; j++)
    {
        new_pcb_ptr->file_descriptor[i].file_ops_table_ptr = NULL; // Mark the rest of the PCB as empty by setting flags to 0 and ops to NULL
        new_pcb_ptr->file_descriptor[i].inode = 0;
        new_pcb_ptr->file_descriptor[i].file_position = 0;
        new_pcb_ptr->file_descriptor[i].flags = 0;
    }

    cur_pid = new_pcb_ptr->pid; // Update the cur_pid, cur_pcb, and the number of processes
    cur_pcb = *new_pcb_ptr;
    num_processes++;

    /* Set up Memory */
    map((void *)USER_SPACE, (void *)(addr_8MB + cur_pid * _4MB)); // sets up the memory by calling map function to map virtual and physical memory
    flush_TLB();                                                  // reset the cr3 value

    /* Read exe Data */
    // 24 offest store into eip 4 bytes
    read_data((uint32_t)cmd_dentry.inode_number, (uint32_t)0, (uint8_t *)PROGRAM_IMG, (uint32_t)_4MB);

    /* Set up old stack and eip */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = addr_8MB - (cur_pid * size_8kb) - 4; // kernel mode stack pointer

    register uint32_t saved_ebp asm("ebp"); // saves the ebp and esp
    register uint32_t saved_esp asm("esp");
    new_pcb_ptr->saved_esp = saved_esp; // sets the pcb to have the saves esp and ebp
    new_pcb_ptr->saved_ebp = saved_ebp;
    /* Go to user mode */
    // will need to set up iret in asm
    iret_setup(cmd_addr); // calls the iret assembly to jump to next process

    return 0;
}

/* TLB */

/* void flush_TLB ()
 *  input   : nothing
 *  output  : nothing
 *  return  : nothing
 *  Description : causes the x86 system to flush cr3 and resets cr3
 */
void flush_TLB()
{ // moving cr3 isntantly flushes cr3, sets cr3 to itself
    asm volatile("                 \n\
            movl    %cr3, %edx      \n\
            movl    %edx, %cr3      \n\
            ");
}

/* Paging Helper */
// use the map helper function

/* void map(void * vaddr, void * paddr)
 *  input   : pointer to virtual and physical memory address
 *  output  : nothing
 *  return  : nothing
 *  Description : maps the virtual memory to physical memory in paging directory.
 */
void map(void *vaddr, void *paddr)
{
    if ((uint32_t)vaddr < 0 || (uint32_t)paddr < 0)
    { // sanity check
        return;
    }
    int pageDirIdx = (uint32_t)vaddr / _4MB; // getting page directory entry indexs

    if (pageDirIdx == USER_PAGE_DIR_IDX)
    {
        // setting page directory at 128 MB virtual address
        paging_directory[pageDirIdx].P = 1;
        paging_directory[pageDirIdx].RW = 1;
        paging_directory[pageDirIdx].US = 1;
        paging_directory[pageDirIdx].PWT = 0;
        paging_directory[pageDirIdx].PCD = 0;
        paging_directory[pageDirIdx].A = 0;
        paging_directory[pageDirIdx].avl = 0;
        paging_directory[pageDirIdx].PS = 1;
        paging_directory[pageDirIdx].AVL = 0;
        paging_directory[pageDirIdx].G = 0;
        paging_directory[pageDirIdx].index_31_12 = (uint32_t)paddr >> 12;
    }
    else
    {
        // setting page directory for video memory page
        paging_directory[pageDirIdx].P = 1;
        paging_directory[pageDirIdx].RW = 1;
        paging_directory[pageDirIdx].US = 1;
        paging_directory[pageDirIdx].PWT = 0;
        paging_directory[pageDirIdx].PCD = 0;
        paging_directory[pageDirIdx].A = 0;
        paging_directory[pageDirIdx].avl = 0;
        paging_directory[pageDirIdx].PS = 1;
        paging_directory[pageDirIdx].AVL = 0;
        paging_directory[pageDirIdx].G = 0;
        paging_directory[pageDirIdx].index_31_12 = 0; // physical page starts at 0
    }
}

/* int32_t sys_read (int32_t fd, void* buf, int32_t nbytes)
 *  input   : fd: entry in file_descriptor array
 *            buf: buf to be filled in
 *            nbytes: number of bytes to be read
 *  output  : none
 *  return  : number of bytes read
 *  Description : read system call
 */
int32_t sys_read(int32_t fd, void *buf, int32_t nbytes)
{
    if (fd < 0 || MAX_FILES <= fd || fd == 1 || buf == NULL || nbytes < 0)
    {
        return -1;
    } // arg checks

    if (cur_pcb.file_descriptor[fd].flags == 0)
    {
        return -1;
    } // Make sure it's been opened

    int bytes_read = 0;

    bytes_read = cur_pcb.file_descriptor[fd].file_ops_table_ptr->read(fd, buf, nbytes); // call to read specified by fd

    cur_pcb.file_descriptor[fd].file_position += bytes_read; // incrementing file position for file read

    return bytes_read;
}

/* int32_t sys_write (int32_t fd, void* buf, int32_t nbytes)
 *  input   : fd: entry in file_descriptor array
 *            buf: buf to be filled in
 *            nybytes: number of bytes to be read
 *  output  : none
 *  return  : number of bytes read
 *  Description : write system call
 */
int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes)
{
    if (fd < 1 || MAX_FILES <= fd || buf == NULL || nbytes < 0)
    {
        return -1;
    } // arg checks

    if (cur_pcb.file_descriptor[fd].flags == 0)
    {
        return -1;
    } // Make sure it's been opened

    int bytes_written = 0;

    bytes_written = cur_pcb.file_descriptor[fd].file_ops_table_ptr->write(fd, buf, nbytes); // call to write specified by fd

    return bytes_written;
}

/* int32_t sys_open (const uint8_t* filename)
 *  input   : filename: name of file we want to open
 *  output  : none
 *  return  : file descriptor index
 *  Description : open system call
 */
int32_t sys_open(const uint8_t *filename)
{
    int res;

    if (filename == NULL)
    { // null check for input
        return -1;
    }

    dentry_t dentry_temp;
    int read_res = read_dentry_by_name(filename, &dentry_temp); // check if filename is in directory

    if (read_res == -1)
    {
        return -1;
    }

    int fd_index = find_next_fd_index(cur_pcb);

    if (fd_index == -1)
    {
        return -1;
    }

    // file type 0 = rtc, 1 = dir, 2 = file
    switch (dentry_temp.file_type)
    {
    case 2:
        res = reg_file.open(filename); // call to file_open

        if (res == -1)
        {
            return -1;
        }

        cur_pcb.file_descriptor[fd_index].file_ops_table_ptr = &(reg_file); // Set to file type
        cur_pcb.file_descriptor[fd_index].inode = res;                      // set inode value in file descriptor entry
        break;
    case 1:
        res = reg_dir.open(filename); // call to directory_open

        if (res == -1)
        {
            return -1;
        }

        cur_pcb.file_descriptor[fd_index].file_ops_table_ptr = &(reg_dir); // Set to dir type
        cur_pcb.file_descriptor[fd_index].inode = 0;                       // inode is set to 0 for rtc and directory
        break;
    case 0:
        res = reg_rtc.open(filename); // call to rtc_open

        if (res == -1)
        {
            return -1;
        }

        cur_pcb.file_descriptor[fd_index].file_ops_table_ptr = &(reg_rtc); // Set to rtc type
        cur_pcb.file_descriptor[fd_index].inode = 0;                       // inode is set to 0 for rtc and directory
        break;
    }
    cur_pcb.file_descriptor[fd_index].file_position = 0;
    cur_pcb.file_descriptor[fd_index].flags = 1; // marks entry in file descriptor array occupied

    return fd_index;
}

/* int32_t sys_close (int32_t fd)
 *  input   : fd: entry in file_descriptor array
 *  output  : none
 *  return  : 0 for success, -1 for failure
 *  Description : close system call
 */
int32_t sys_close(int32_t fd)
{
    if (fd < 2 || MAX_FILES <= fd)
    {
        return -1;
    } // can't close stdin or stdout
    if (cur_pcb.file_descriptor[fd].flags == 0)
    {
        return -1;
    }
    cur_pcb.file_descriptor[fd].flags = 0; // Set fd availability flag to 0
    cur_pcb.file_descriptor[fd].file_position = 0;
    cur_pcb.file_descriptor[fd].inode = -1;
    cur_pcb.file_descriptor[fd].file_ops_table_ptr->close(fd); // call close function for file descriptor
    return 0;
}

/* int get_PCB_addr()
 *  input   : none
 *  output  : none
 *  return  : address of current PCB in the form of integer
 *  Description : helper function that gets the PCB address
 */
int get_PCB_addr()
{
    return addr_8MB - (size_8kb * (cur_pid + 1));
}

/* pcb_t get_cur_PCB
 *  input   : none
 *  output  : none
 *  return  : returns pcb_t object for the current pcb
 *  Description : helper function that gets the current pcb
 */
pcb_t get_cur_PCB()
{
    return cur_pcb;
}

/* int find_next_fd_index(pcb_t p)
 *  input   : p: a pcb object
 *  output  : index into file descriptor or -1
 *  return  : next avaliable file descriptor
 *  Description : helper function that gets the next avaliable file descriptor
 */
int find_next_fd_index(pcb_t p)
{
    int i;
    for (i = 2; i < MAX_FILES; i++)
    { // go through file_descriptor and see if any entries are open
        if (p.file_descriptor[i].flags == 0)
        {
            return i; // open entry was found
        }
    }
    return -1; // entry was not found
}

/* int32_t sys_getargs
 *  input   : buf: buffer to be filled | nbytes: number of bytes to be read
 *  output  : status of sys_getargs()
 *  return  : 0 for success, -1 for failure
 *  Description : function that puts arg into buf
 */
int32_t sys_getargs(uint8_t *buf, int32_t nbytes)
{
    if (buf == NULL)
    { // null checks
        return -1;
    }
    if (nbytes < 0)
    {
        return -1;
    }
    if (cur_pcb.arg[0] == '\0')
    {
        return -1;
    }
    if (cur_pcb.arg_len > KEY_BUFF_SIZE + 1)
    {
        return -1;
    }
    int i;
    // copying over arg into buf
    for (i = 0; i < nbytes; i++)
    {
        if (cur_pcb.arg[i] == '\0')
        {
            break;
        }
        buf[i] = cur_pcb.arg[i];
    }
    buf[i] = '\0';

    return 0;
}

/* int32_t sys_vidmap
 *  input   : screen_start: pointer to pointer of video memory
 *  output  : status of sys_vidmap, *(screen_start) is modified
 *  return  : 0 for success, -1 for failure
 *  Description : function that maps virtual video memory
 */
int32_t sys_vidmap(uint8_t **screen_start)
{
    if (screen_start == NULL)
    {
        return -1;
    } // null checks
    if ((int)screen_start < USER_SPACE || (int)screen_start > (USER_SPACE + _4MB))
    {
        return -1;
    }

    *screen_start = (uint8_t *)VID_MEM_ADDR;    // set *(screen_start) to virtual address of video memory
    map((void *)(VID_MEM_DIR), (void *)(0x00)); // physical address of directory where vid mem is located is 0
    flush_TLB();

    return 0;
}

// CP 5 maybe?
int32_t sys_set_handler(int32_t signum, void *handler_address) { return -1; }
int32_t sys_sigreturn(void) { return -1; }

// Layout Scheduling
void scheduling()
{
    cli(); // stop interrupts
    /* Set next terminal by cycling with main terminal (main + 1) % 3 */

    /* Set next pid with the next main terminal's pid */

    /* Save ebp and esp. Double check position with local variables*/

    /* Update main terminal fields (pid, ebp, esp) */
    // only if (flag != 0)

    /* For first 3 processes/shells */
    // if (flag < 3)
    // set main terminal's eoi flag to 0
    // set video page

    // if (flag != 0)
    // copy over B8 to BA+main (* use strncpy for eol)
    // copy over BA+next to B8

    // set display terminal to the next terminal
    // set main terminal to the next terminal

    // Increment flag
    // sys execute shell and return

    /* Update esp and ebp using in line assembly. Use next terminal's saved esp and ebp */

    /* Switch video mapping */
    // Flush TLB

    // if (next terminal == main terminal) -> break
    // else if (next terminal == display terminal && main terminal != display terminal)
    // copy vidmem to 0xBA+main
    // set video page
    // remap 0xB8 page to physical mem (page_table[ENTRIES]31_12 = 0xB8)

    // else if (next terminal != display terminal && main terminal == display terminal)
    // copy vidmem to 0xBA+main
    // Set inv video page. Same as set video page except 31_12 = (next terminal + 8) << 10
    // Flush TLB
    // copy BA+next to vidmem
    // remap 0xB8 page to physical mem (page_table[ENTRIES]31_12 = ((8+next)*4MB + 184*4KB) / 4KB)

    /* Update pid and main*/
    // cur pid = next pid
    // main terminal = next terminal

    /* TSS */
    // ss0 = KERNEL_DS
    // esp0 = 8MB - (8KB * cur pid) - 4

    /* To user code */
    // set page dir 32's 31_12 = (8 + next pid) << 10. Similar to map function

    // Flush TLB

    /* Update cursor with display terminal */
    // call update_cursor using display terminal

    // send_eoi(0)
    return;
}
