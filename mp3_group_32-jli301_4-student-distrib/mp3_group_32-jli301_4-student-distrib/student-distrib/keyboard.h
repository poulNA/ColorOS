/* keyboard.h - Defines used in interactions with the keyboard interrupts
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#ifndef ASM

#include "types.h"
#include "lib.h"
#include "i8259.h"

#define KEYBOARD_IRQ    1
#define KEYBOARD_DATA_PORT   0x60

#define SCANCODE_SIZE       59
#define KEY_BUFF_SIZE       127 // leaving 1 open space for new line '\n'
#define NUM_SPECIAL         8

/* Special Keyboard Inputs*/

#define ESC         0x01
#define BACKSPACE   0x0E
#define TAB         0x0F    
#define ENTER       '\n'
#define CTRL        0x1D
#define LSHIFT      0x2A
#define RSHIFT      0x36
#define ALT         0x38
#define SPACE       0x39
#define CAPSL       0x3A
#define SHIFT_LEFT_RELEASE   0xAA
#define SHIFT_RIGHT_RELEASE   0xB6
#define CTRL_RELEASE    0x9D
#define ENTER_PRESS 0x1C
#define TAB_SIZE 4
#define LETTER_NUM 26
#define L_KEY 0x26


extern int capsChar;
extern int shiftChar;
extern int ctrlFlag;
extern int altFlag;
extern int ctrlIndex;
extern unsigned char keyboard_buffer[KEY_BUFF_SIZE];
extern int keyIndex;
extern int enterPress;
extern int tabIndex;


/* Initialize Keyboard */
void keyboard_init(void);
/* Handle Keyboard Inputs */
void keyboard_input(void);
/* Handle Keyboard special characters */
int keyboard_special(uint8_t key_pressed);
/* Handle Keyboard buffer*/
// void keyboard_buffer_output(void);

void resetBuff(void);

#endif

#endif /* _KEYBOARD_H */
