#include <xc.h>
#include <stdint.h>
#include "LCD.h"
#include "PIC18F46K22-Config.h"

#define MAX_ATTEMPTS 3

uint8_t counter = 0;
uint8_t lockout = 0;

// Called ONLY after # is pressed and
// a complete 5-digit PIN is submitted.
void checkPassword(uint8_t passwordCorrect)
{
    if (passwordCorrect)
    {
        counter = 0;

        // Correct password
        // Servo unlock handled later
    }
    else
    {
        counter++;

        LCD_cursor_set(1, 1);
        LCD_write_string("Incorrect PIN");

        LCD_cursor_set(2, 1);
        LCD_write_string("Try Again");

        __delay_ms(1000);
        LCD_clear();

        if (counter >= MAX_ATTEMPTS)
        {
            lockout++;
            counter = 0;

            LCD_cursor_set(1, 1);
            LCD_write_string("System Locked");

            LCD_cursor_set(2, 1);
            LCD_write_string("Wait 30 Seconds");

            // Lockout for 30 seconds
            for (uint8_t i = 0; i < 30; i++)
            {
                __delay_ms(1000);
            }

            LCD_clear();
        }
    }
}