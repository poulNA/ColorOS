/* keyboard.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

/* Flags for the special character and indexes to tabing and ctrl*/
int capsChar;
int shiftChar;
int ctrlFlag;
int altFlag;
int ctrlIndex;
unsigned char keyboard_buffer[KEY_BUFF_SIZE];
int keyIndex;
int enterPress;
int shiftCount;
int tabIndex; 
int capsSpecialFlag;
int specialFlag;
//unsigned char special[NUM_SPECIAL] = { ESC, BACKSPACE, TAB, ENTER, CTRL, RSHIFT, ALT, CAPSL};


/* This scanLetter Matrix is used for shift and capslocks. We itterate though it each keyboard char press to set the capsSpecialFlag
 * to 1 if it is a special character and 0 if it is a letter. The capsSpecialFlag is used for the shift and caps logic
 */
unsigned char scanLetter[LETTER_NUM][2] = { 
        {'q', 'Q'},
        {'w', 'W'},
        {'e', 'E'},
        {'r', 'R'}, 
        {'t', 'T'},
        {'y', 'Y'},
        {'u', 'U'},
        {'i', 'I'},
        {'o', 'O'},
        {'p', 'P'}, 
        {'a', 'A'},
        {'s', 'S'},
        {'d', 'D'},
        {'f', 'F'},
        {'g', 'G'},
        {'h', 'H'},
        {'j', 'J'},
        {'k', 'K'},
        {'l', 'L'},  
        {'z', 'Z'},
        {'x', 'X'},
        {'c', 'C'},   
        {'v', 'V'},
        {'b', 'B'},
        {'n', 'N'},  
        {'m', 'M'},
};



/* 
 * The scanCode 2D array is filled, on the left, with the ASCII character that corresponds to its index. 
 * The index value is equivalent to the universal scancode 1 keyboard. For Checkpoint 1, to get the 
 * lowercase values and numbers, only the scanCode[ASCII Index][0] is in use, the [][1] index is unused. 
 */

unsigned char scanCode[SCANCODE_SIZE][2] = {
        {'0', '0'},
        {ESC, ESC},
        {'1', '!'},
        {'2', '@'},
        {'3', '#'},
        {'4', '$'},
        {'5', '%'},
        {'6', '^'},
        {'7', '&'},
        {'8', '*'},
        {'9', '('},
        {'0', ')'},
        {'-', '_'},
        {'=', '+'},
        {BACKSPACE, BACKSPACE},
        {TAB, TAB}, 
        {'q', 'Q'},
        {'w', 'W'},
        {'e', 'E'},
        {'r', 'R'}, 
        {'t', 'T'},
        {'y', 'Y'},
        {'u', 'U'},
        {'i', 'I'},
        {'o', 'O'},
        {'p', 'P'},      
        {'[', '{'}, 
        {']', '}'},
        {ENTER, ENTER},
        {CTRL, CTRL},
        {'a', 'A'},
        {'s', 'S'},
        {'d', 'D'},
        {'f', 'F'},
        {'g', 'G'},
        {'h', 'H'},
        {'j', 'J'},
        {'k', 'K'},
        {'l', 'L'},
        {';', ':'},
        {0x27, '"'},  // 0x27 is the hex code value for ' since it breaks the char
        {'`', '~'},
        {LSHIFT, LSHIFT},
        {92, '|'},   // 92 is the ascii code value for \ since it breaks the char
        {'z', 'Z'},
        {'x', 'X'},
        {'c', 'C'},   
        {'v', 'V'},
        {'b', 'B'},
        {'n', 'N'},  
        {'m', 'M'},
        {',', '<'},
        {'.', '>'},   
        {'/', '?'},
        {RSHIFT, RSHIFT},
        {NULL, NULL},  
        {ALT, ALT},
        {' ', ' '},  
        {CAPSL, CAPSL},
};

/* function     : keyboard_init
 * input        : nothing
 * output       : nothing
 * Description  : This function enables the use of the keyboard on the i8259 pic. Allows for keyboard interrupts.
 * return       : nothing
 */
void keyboard_init(void){
    capsChar = 0;                                           // Initializing all the flags and index to 0 at start of interrupts
    shiftChar = 0;
    keyIndex = 0;
    tabIndex = 0;
    capsSpecialFlag = 0;
    specialFlag = 0;
    ctrlFlag = 0;
    altFlag = 0;
    ctrlIndex = 0;
    enterPress = 0;
    keyboard_buffer[KEY_BUFF_SIZE] = ENTER;
    enable_irq(KEYBOARD_IRQ);                               // enables an interrupt to be read on the PIC at the keyboard IRQ 
}



/* function     : keyboard_input
 * input        : nothing
 * output       : ASCII character to the screen.
 * Description  : This function outputs the corresponding ASCII char onto the screen that was typed by the user on the keyboard.
 *                It reads from the keyboard data port, searches for the corresponding char in the scancode dictionary, then 
 *                prints it to the screen.
 * return       : nothing
 */
/* Handle Keyboard Inputs */
void keyboard_input(void){  
    int i;

    uint8_t key_pressed = inb(KEYBOARD_DATA_PORT) & 0xFF;   // the data port with 1111 1111 to keep the last 8 bit value. 

    // for (i = 0; i < NUM_SPECIAL; i++) { 
    //     if (key_pressed == special[i]) {specialFlag = 1;}
    //     else { specialFlag = 0;}
    // }
    // j = keyboard_special(key_pressed);
    
    if (key_pressed == NULL){                                                                   // exists if invalid key press
        return;
    } else if (key_pressed == SHIFT_RIGHT_RELEASE || key_pressed == SHIFT_LEFT_RELEASE){        // changes shift flags upon release of shift key
        shiftChar = 0;
    } else if (key_pressed == CTRL_RELEASE){                                                    // changes ctrl flag upon ctrl key release
        ctrlFlag = 0;
    }

    if (keyIndex >= KEY_BUFF_SIZE && !enterPress){                                              // if buffer is full with no enter, allows user to 
        if (key_pressed == BACKSPACE) {                                                         // backspace into the buffer and write until 
                if (tabIndex == keyIndex) {                                                     // buffer is full again or enter is pressed
                    for (i = 0; i < TAB_SIZE; i++) {                                                   // backspace tab check (tab is 4 spaces)
                        backspace(); 
                        keyIndex--;
                    }
                } else {                                                                        // regular backspace 
                    backspace(); 
                    keyIndex--;
                }
        }
    }

    if ( keyIndex <= KEY_BUFF_SIZE /*&& !enterPress*/) {                                        // checks that the index is within buffer size
        if (key_pressed <= CAPSL) {                                                              // check valid key in scancode
            if (key_pressed == ENTER_PRESS) {                                                          // sets enter flag, writes enter to buffer
                enterPress = 1;
                putc('\n');                                                                     // calls putc for enter 
                keyboard_buffer[keyIndex] = '\n';
                keyIndex++;
            } else if (key_pressed == ALT){                                                     // sets alt flag
                // for (i = 0; i < keyIndex; i++ ) { putc(keyboard_buffer[i]); }
                altFlag = 1;
            } else if (key_pressed == BACKSPACE && keyIndex > 0) {                                              // calls backspace, function , reset the key index
                // keyIndex--;
                if (keyboard_buffer[keyIndex-1] == TAB) {                                                     // if tab was right before backspace, it backspaces on the tab
                    for (i = 0; i < TAB_SIZE; i++) {
                        backspace(); 
                    }
                } else {                                                                        // regular backspace
                    backspace(); 
                }
                keyIndex--;
            } else if (key_pressed == LSHIFT || key_pressed == RSHIFT) {                        //sets shift flag if shift is pressed
                shiftChar = 1;
            } else if (key_pressed == CAPSL) {                                                  //toggles caps flag on and off 
                //printf("caps");
                if (capsChar == 1) { capsChar = 0; } else { capsChar = 1; }
            } else if (key_pressed == ESC) {                                                    //esc doesn't do anything, sets alt flag
                altFlag = 1;
            } else if (key_pressed == TAB) {                                                    // calls tab in putc, keys track of the index in buffer
                tabIndex = keyIndex;
                keyboard_buffer[keyIndex] = TAB;
                keyIndex++;
                // keyIndex += 4;
                putc(TAB);                                                                      // calls the tab in putc
            } else if (key_pressed == CTRL) {                                                   // sets the ctrl index and flag
                ctrlFlag = 1;
                ctrlIndex = keyIndex;
            } else {

                for (i = 0; i < LETTER_NUM; i++){                                                       // iterrates through the letter scan code 
                    if (scanLetter[i][1] == scanCode[key_pressed][1]){                          
                        capsSpecialFlag = 0;                                                    // sets cpasSpecialFlag to 1 if not a letter
                        break;
                    } 
                    capsSpecialFlag = 1;
                }

                if (ctrlFlag && key_pressed == L_KEY && ctrlIndex == keyIndex) {                                     // sets ctrl+l : clears the screen an resets cursor
                    keyboard_buffer[keyIndex] = scanCode[key_pressed][0];                                           // adds l to buffer, but clears the keyboard buffer after
                    clear();
                    resetBuff();                                                                                    // reset buffer and flags
                    ctrlFlag = 0;
                    keyIndex = 0;
                } else if (ctrlFlag && key_pressed == L_KEY && ctrlIndex == keyIndex && shiftChar) {                 // case: ctrl+shift+l aka ctrl+L
                    keyboard_buffer[keyIndex] = scanCode[key_pressed][0];
                    clear();
                    resetBuff();
                    ctrlFlag = 0;
                    keyIndex = 0;
                } else if (!specialFlag && key_pressed != BACKSPACE){
                    if (capsChar && shiftChar && !capsSpecialFlag) {                                                // caps logic for shift and unshift combinations for all keyboard keys
                        keyboard_buffer[keyIndex] = scanCode[key_pressed][0];                                       // does not allow special characters if both caps and shift held
                        putc(scanCode[key_pressed][0]); 
                        capsSpecialFlag = 0;
                    } else if (capsChar && shiftChar && capsSpecialFlag) {                                          // allows for special chars if cpas is on but shift is held down
                        keyboard_buffer[keyIndex] = scanCode[key_pressed][1];
                        putc(scanCode[key_pressed][1]);
                        capsSpecialFlag = 0;
                    } else if (capsChar && capsSpecialFlag && !shiftChar) {                                         // does not allow special characters if caps but not shift
                        keyboard_buffer[keyIndex] = scanCode[key_pressed][0];
                        putc(scanCode[key_pressed][0]);
                        capsSpecialFlag = 0;
                    } else if (capsChar || shiftChar) {                                                             // prints upper case on letters
                        keyboard_buffer[keyIndex] = scanCode[key_pressed][1];
                        putc(scanCode[key_pressed][1]);
                    } else {                                                                                        //prints lower case
                        keyboard_buffer[keyIndex] = scanCode[key_pressed][0];
                        putc(scanCode[key_pressed][0]);
                    }
                    keyIndex++;
                }
            }
        }
    }
    
    send_eoi(KEYBOARD_IRQ);                                 // send eoi to the interrupt controller
}

void resetBuff(void){                                       // resets the buffer
    enterPress = 0;
    int i;
    for (i = 0; i < KEY_BUFF_SIZE; i++){                    // fills buffer with 0
        keyboard_buffer[i] = 0;
    }
    keyIndex = 0;                                           // reset keyIndex to 0
}


