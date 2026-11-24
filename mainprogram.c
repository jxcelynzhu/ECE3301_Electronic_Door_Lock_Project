
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "LCD.h"
#include "PIC18F46K22-Config.h"

#define _XTAL_FREQ 16000000UL


// SYSTEM CONFIGURATION

#define PASSWORD_LENGTH 4
#define MAX_ATTEMPTS 3

// EEPROM memory addresses
#define MARKER_ADDRESS 0x00
#define PASSWORD_ADDRESS 0x01
#define PASSWORD_MARKER 0xA5

// Keypad configuration
char keys[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

// Password variables
char savedPassword[PASSWORD_LENGTH + 1] = {0};
char enteredPassword[PASSWORD_LENGTH + 1] = {0};

uint8_t digitCount = 0;
uint8_t counter = 0;
bool passwordCreated = false;



// FUNCTION DECLARATIONS


void hardware_init(void);

char keypad_get_key(void);

void getPassword(void);
void createPassword(void);
void enterPassword(void);

void savePassword(void);
void loadPassword(void);

void EEPROM_Write(uint16_t address, uint8_t data);
uint8_t EEPROM_Read(uint16_t address);

void EEPROM_WriteBlock(uint16_t address,
                       const void *data, size_t len);

void EEPROM_ReadBlock(uint16_t address,
                      void *data, size_t len);

void checkPassword(void);
void system_lockout(void);
void door_unlock(void);


// HARDWARE INITIALIZATION


void hardware_init(void)
{
    // Configure keypad pins
    TRISB = 0xE0;

    // Set PORTB to digital I/O
    ANSELB = 0x00;

    // Enable PORTB weak pull-ups on keypad columns
    INTCON2bits.RBPU = 0;

    WPUBbits.WPUB5 = 1;
    WPUBbits.WPUB6 = 1;
    WPUBbits.WPUB7 = 1;

    // Initialize keypad rows
    LATB = 0x1E;

    // Configure PORTD as digital outputs
    TRISD = 0x00;
    ANSELD = 0x00;
    LATD = 0x00;

    // LCD initialization
    

    LCD_clear();

    LCD_cursor_set(1, 1);
    LCD_write_string("Door Lock System");

    LCD_cursor_set(2, 1);
    LCD_write_string("Initializing...");

    __delay_ms(1500);

    LCD_clear();
}



// KEYPAD SCANNING


char keypad_get_key(void)
{
    char pressedKey = 0;

    for (uint8_t row = 0; row < 4; row++)
    {
        // Set all keypad rows HIGH
        LATB = 0x1E;

        // Set selected row LOW
        if (row == 0)
        {
            LATBbits.LATB1 = 0;
        }
        else if (row == 1)
        {
            LATBbits.LATB2 = 0;
        }
        else if (row == 2)
        {
            LATBbits.LATB3 = 0;
        }
        else
        {
            LATBbits.LATB4 = 0;
        }

        __delay_us(20);

        // Check keypad columns
        if (PORTBbits.RB5 == 0)
        {
            pressedKey = keys[row][0];
        }
        else if (PORTBbits.RB6 == 0)
        {
            pressedKey = keys[row][1];
        }
        else if (PORTBbits.RB7 == 0)
        {
            pressedKey = keys[row][2];
        }

        if (pressedKey != 0)
        {
            break;
        }
    }

    // Restore all rows HIGH
    LATB = 0x1E;

    if (pressedKey != 0)
    {
        // Debounce
        __delay_ms(30);

        // Wait until key is released
        while (PORTBbits.RB5 == 0 ||
               PORTBbits.RB6 == 0 ||
               PORTBbits.RB7 == 0)
        {
            // Wait
        }

        __delay_ms(30);
    }

    return pressedKey;
}



// RECEIVE PASSWORD FROM USER


void getPassword(void)
{
    digitCount = 0;

    memset(enteredPassword, 0, sizeof(enteredPassword));

    while (1)
    {
        char key = keypad_get_key();

        if (key == 0)
        {
            continue;
        }

        // User enters a number
        if (key >= '0' && key <= '9')
        {
            if (digitCount < PASSWORD_LENGTH)
            {
                enteredPassword[digitCount] = key;

                digitCount++;
            }
        }

        // Clear current password entry
        else if (key == '*')
        {
            digitCount = 0;

            memset(enteredPassword, 0,
                   sizeof(enteredPassword));

            LCD_clear();

            LCD_cursor_set(1, 1);
            LCD_write_string("PIN Cleared");

            __delay_ms(500);

            LCD_clear();
        }

        // Submit password
        else if (key == '#')
        {
            if (digitCount == PASSWORD_LENGTH)
            {
                enteredPassword[PASSWORD_LENGTH] = '\0';

                return;
            }

            else
            {
                LCD_clear();

                LCD_cursor_set(1, 1);
                LCD_write_string("Enter 4 Digits");

                __delay_ms(1000);

                LCD_clear();
            }
        }
    }
}



// CREATE INITIAL PASSWORD


void createPassword(void)
{
    LCD_clear();

    LCD_cursor_set(1, 1);
    LCD_write_string("Create New PIN");

    LCD_cursor_set(2, 1);
    LCD_write_string("Enter 4 Digits");

    getPassword();

    // Copy entered password into saved password
    strcpy(savedPassword, enteredPassword);

    // Store password in EEPROM
    savePassword();

    passwordCreated = true;

    LCD_clear();

    LCD_cursor_set(1, 1);
    LCD_write_string("PIN Saved!");

    LCD_cursor_set(2, 1);
    LCD_write_string("System Ready");

    __delay_ms(1500);

    LCD_clear();
}



// SAVE PASSWORD TO EEPROM


void savePassword(void)
{
    // Write password into EEPROM
    EEPROM_WriteBlock(
        PASSWORD_ADDRESS,
        savedPassword,
        PASSWORD_LENGTH
    );

    // Write marker LAST to indicate valid password
    EEPROM_Write(
        MARKER_ADDRESS,
        PASSWORD_MARKER
    );
}



// LOAD PASSWORD FROM EEPROM


void loadPassword(void)
{
    // Check if password was previously created
    uint8_t marker = EEPROM_Read(MARKER_ADDRESS);

    if (marker == PASSWORD_MARKER)
    {
        // Retrieve saved password
        EEPROM_ReadBlock(
            PASSWORD_ADDRESS,
            savedPassword,
            PASSWORD_LENGTH
        );

        savedPassword[PASSWORD_LENGTH] = '\0';

        passwordCreated = true;
    }

    else
    {
        passwordCreated = false;
    }
}



// ENTER PASSWORD TO UNLOCK DOOR


void enterPassword(void)
{
    LCD_clear();

    LCD_cursor_set(1, 1);
    LCD_write_string("Enter Password");

    LCD_cursor_set(2, 1);
    LCD_write_string("Press # to Enter");

    getPassword();

    checkPassword();
}


// CHECK PASSWORD


void checkPassword(void)
{
    // Compare entered password with saved password
    if (strcmp(enteredPassword, savedPassword) == 0)
    {
        // Correct password
        counter = 0;

        LCD_clear();

        door_unlock();
    }

    else
    {
        // Incorrect password
        counter++;

        LCD_clear();

        LCD_cursor_set(1, 1);
        LCD_write_string("Incorrect PIN");

        LCD_cursor_set(2, 1);
        LCD_write_string("Try Again");

        __delay_ms(1500);

        LCD_clear();

        // Check maximum attempts
        if (counter >= MAX_ATTEMPTS)
        {
            counter = 0;

            system_lockout();
        }
    }
}



// SYSTEM LOCKOUT


void system_lockout(void)
{
    LCD_clear();

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

    LCD_cursor_set(1, 1);
    LCD_write_string("Lockout Ended");

    __delay_ms(1000);

    LCD_clear();
}



// DOOR UNLOCK FUNCTION


void door_unlock(void)
{
    LCD_cursor_set(1, 1);
    LCD_write_string("Access Granted");

    LCD_cursor_set(2, 1);
    LCD_write_string("Door Unlocked");


    // Add servo motor control here

    __delay_ms(3000);

   
    // Return servo to locked position

    LCD_clear();
}



// EEPROM WRITE FUNCTION


void EEPROM_Write(uint16_t address, uint8_t data)
{
    // Wait for previous write cycle
    while (EECON1bits.WR);

    // Load EEPROM address
    EEADRH = (uint8_t)((address >> 8) & 0x03);
    EEADR = (uint8_t)(address & 0xFF);

    // Load data
    EEDATA = data;

    // Select DATA EEPROM
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;

    // Enable writes
    EECON1bits.WREN = 1;

    // Save interrupt state
    uint8_t gie_state = INTCONbits.GIE;

    // Disable interrupts
    INTCONbits.GIE = 0;

    // Required EEPROM unlock sequence
    EECON2 = 0x55;
    EECON2 = 0xAA;

    // Begin write
    EECON1bits.WR = 1;

    // Restore previous interrupt state
    INTCONbits.GIE = gie_state;

    // Wait for write completion
    while (EECON1bits.WR);

    // Disable EEPROM writes
    EECON1bits.WREN = 0;
}


// EEPROM READ FUNCTION


uint8_t EEPROM_Read(uint16_t address)
{
    // Wait for ongoing write
    while (EECON1bits.WR);

    // Load address
    EEADRH = (uint8_t)((address >> 8) & 0x03);
    EEADR = (uint8_t)(address & 0xFF);

    // Select DATA EEPROM
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;

    // Start read
    EECON1bits.RD = 1;

    return EEDATA;
}



// EEPROM WRITE BLOCK


void EEPROM_WriteBlock(
    uint16_t address,
    const void *data,
    size_t len
)
{
    const uint8_t *bytes = (const uint8_t *)data;

    for (size_t i = 0; i < len; i++)
    {
        // Only write if data has changed
        if (EEPROM_Read(address + i) != bytes[i])
        {
            EEPROM_Write(address + i, bytes[i]);
        }
    }
}



// EEPROM READ BLOCK


void EEPROM_ReadBlock(
    uint16_t address,
    void *data,
    size_t len
)
{
    uint8_t *bytes = (uint8_t *)data;

    for (size_t i = 0; i < len; i++)
    {
        bytes[i] = EEPROM_Read(address + i);
    }
}



// MAIN PROGRAM


void main(void)
{
    // Initialize hardware
    hardware_init();

    // Retrieve saved password from EEPROM
    loadPassword();

    // If no password exists, create one
    if (!passwordCreated)
    {
        createPassword();
    }

    // Main door lock operation
    while (1)
    {
        // Ask user to enter password
        enterPassword();
    }
}

