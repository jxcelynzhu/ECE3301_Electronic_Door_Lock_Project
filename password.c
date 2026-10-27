
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PASSWORD_LENGTH 4

// Store password in RAM
char savedPassword[5] = {0};

// Store digits currently being entered
char enteredPassword[5] = {0};

uint8_t digitCount = 0;
bool passwordCreated = false;

void hardware_init(void);
char keypad_get_key(void);

void processKey(char key)
{
    // User enters a digit (0-9)
    if (key >= '0' && key <= '9')
    {
        if (digitCount < PASSWORD_LENGTH)
        {
            enteredPassword[digitCount] = key;
            digitCount++;
        }
    }

    // Press '*' to reset attempt
    else if (key == '*')
    {
        digitCount = 0;
    }

    // Press '#' to save password
    else if (key == '#')
    {
        if (digitCount == PASSWORD_LENGTH &&
            passwordCreated == false)
        {
            enteredPassword[4] = '\0';

            // Copy entered PIN into RAM
            strcpy(savedPassword, enteredPassword);
            passwordCreated = true;
        }

        // Reset for next input
        digitCount = 0;
    }
}

void main(void)
{
    hardware_init();

    while (1)
    {
        char key = keypad_get_key();

        if (key != 0)
        {
            processKey(key);
        }
    }
}
