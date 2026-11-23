#include <xc.h>
#include <stdint.h>
#include <stddef.h>

void EEPROM_Write(uint16_t address, uint8_t data){
    
    // wait for previous write cycles to complete
    while(EECON1bits.WR);
    
    // load 10 bit address into EEADR and EEADRH Registers
    EEADRH = (uint8_t)((address >> 8) & 0x03);  // extract bits 8 and 9 of address
    EEADR = (uint8_t)(address & 0xFF);  // extract bits 1-7 of address
    
    EEDATA = data;  // data to be written
    
    // point to EECON1, EEPGD: DATA EEPROM memory
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    
    // enable writes
    EECON1bits.WREN = 1;
    
    // save global interrupt state
    uint8_t gie_state = INTCONbits.GIE;  //GIE = global interrupts enable
    // disable interrupts
    INTCONbits.GIE = 0;
    
    //  required sequence per PIC18F46K22 Datasheet
    EECON2 = 0x55;
    EECON2 = 0xAA;
    
    // initiate write cycle
    EECON1bits.WR = 1;
    
    // restore GIE
    INTCONbits.GIE = 1;
    
    // disable writes
    EECON1bits.WREN = 0;
}

uint8_t EEPROM_Read(uint16_t address){
    // wait for ongoing write cycles to finish
    while (EECON1bits.WR);
    
    // load address to read
    EEADRH = (uint8_t)((address >> 8) & 0x03);
    EEADR = (uint8_t)(address & 0xFF);
    
    // select DATA EEPROM memory
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    
    // initiate read
    EECON1bits.RD = 1;
    
    return EEDATA;
}

void EEPROM_WriteBlock(uint16_t address, const void *data, size_t len){
    const uint8_t *bytes = (const uint8_t *)data;  // this is the memory address of the byte
    for (size_t i = 0; i < len; i++){
        // only write if byte is changed to reduce EEPROM wear
        if (EEPROM_Read(address + i) != bytes[i]){
            EEPROM_Write(address + i, bytes[i]);
        }
    }
}

void EEPROM_ReadBlock(uint16_t address, void *data, size_t len){
    uint8_t *bytes = (uint8_t *)data;
    for (size_t i = 0; i < len; i++){
        bytes[i] = EEPROM_Read(address + i);
    }
}
