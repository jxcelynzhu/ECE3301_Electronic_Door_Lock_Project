#include <xc.h>
#include "Configuration.h"

//unsigned char keys[4][3]={{1,2,3},{4,5,6},{7,8,9},{'*',0,'#'}}; // 2D array for keypad matrix

// Replaced non-integer characters with numbers for LED test
unsigned char keys[4][3]={{1,2,3},{4,5,6},{7,8,9},{14,0,15}}; 


void main(void) {
    TRISB = 0xE0; // 0b11100000, determining input & output pins
    ANSELB = 0; // Setting PORTB pins as digital I/O
    
    // Using PORTC for test LEDs
    TRISC = 0; // PORTC pins = outputs
    ANSELC = 0; // PORTC pins are digital
    LATC = 0x00; // Driving all PORTC pins LOW
    
    char user_password[4]; // Stores the entered 4-digit password combination
    
    while(1){
        // Detecting selected key
        for (int row=0; row<4; row++){
            
            // Initialize all rows to HIGH
            LATBbits.LATB1 = 1;  
            LATBbits.LATB2 = 1;
            LATBbits.LATB3 = 1;
            LATBbits.LATB4 = 1;
            
            // Setting selected row to LOW
            if (row == 0) {
                LATBbits.LATB1=0;
            }
            else if (row == 1){
                LATBbits.LATB2=0;

            }
            else if (row == 2){
                LATBbits.LATB3=0;

            }
            else{
                LATBbits.LATB4=0; 
            }
            
            // Checking which column is LOW
            int col = 3;
            if (PORTBbits.RB5 == 0) {
                col = 0;
            }
            else if (PORTBbits.RB6 == 0) {
                col = 1;
            }
            else if (PORTBbits.RB7 == 0) {
                col = 2;
            }
            
            // Storing selected key
            if (col != 3) {
                unsigned char selected_key = keys[row][col]; // Stores value of the selected key
                LATC = selected_key; // Binary representation of selected key is shown on the LEDs
            }
        }
        
    }
    return;
}
