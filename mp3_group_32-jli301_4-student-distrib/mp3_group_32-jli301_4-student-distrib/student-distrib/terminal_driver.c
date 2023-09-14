/* terminal_driver.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

#define OS_SIZE 6 // size of "391OS>"

int start; // global variables for start and end
int end;

/* function     : terminal_init
 * input        : nothing
 * output       : nothing
 * Description  : Clears screen and initializes terminal global variables
 * return       : nothing
 */
void terminal_init(void){
    clear(); // clear screen and initializes start/end to 0
    start = 0;
    end = 0;
}

/* function     : terminal_rad
 * input        : fd, buf, nbytes
 * output       : passed in buffer is filled
 * Description  : Reads keyboard buffer and fills passed in buffer
 * return       : number of bytes read
 */
int terminal_read(int32_t fd, void* buf, int32_t nbytes){
    int i;
    int endflag = 0; // indicates end of buffer
    if (fd != 0) { return -1;} // null checks for parameter
    if (buf == NULL) { return -1; }

    while(1){
        // cli();
        for (i = 0; i < keyIndex; i++) { 
            if (keyIndex <= KEY_BUFF_SIZE && keyboard_buffer[i] != '\n') // checking if enter key was pressed
            {
                ((char*)buf)[i] = keyboard_buffer[i];
            }
            if (keyboard_buffer[i] == '\n') {
                end = i;
                endflag = 1;
                break;
            }
        }
        
        // sti();
        if (endflag) {break;} 
    }
    int ret = end; // set ret and start after loop
    start += ret + 1;
    // resetBuff();
    return ret;

}

/* function     : terminal_write
 * input        : fd, buf, nbytes
 * output       : buffer is echoed in terminal
 * Description  : Takes in buffer and writes it to the screen
 * return       : number of bytes written
 */
int terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    int i;
    if (fd != 1) { return -1;} // null checks for parameters
    if (buf == NULL) { return -1; }

    char OS_BUF[OS_SIZE] = "391OS>"; // 391OS> buf

    for(i = 0; i < OS_SIZE; i++){ // checking if buf is equal to '391OS>'
        if(((char*)buf)[i] != OS_BUF[i]){
            break;
        }
    }

    if(i == 6) // if buf is equal to '391OS>' print a new line
        putc(ENTER);
    
    for (i = 0; i < nbytes; i++){ // write buf to screen
        putc((uint8_t) ((char*)buf)[i]);
    }

    resetBuff(); // reset the keyboard buffer
    return nbytes;
}

/* function     : terminal_open
 * input        : filename
 * output       : 0
 * Description  : Opens terminal (does nothing for checkpoint 2)
 * return       : 0
 */
int terminal_open(const uint8_t* filename){
    if(filename == NULL)
        return -1;
    return 0;
}

/* function     : terminal_close
 * input        : fd
 * output       : 0
 * Description  : Closes terminal and resets keyboard buffer
 * return       : 0
 */
int terminal_close(int32_t fd){
    if(fd < 0)
        return -1;
    resetBuff(); // resets keyboard buffer
    return 0;
}
