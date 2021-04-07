#include <stdio.h>
#include <stdint.h>   // for uint8_t and uint16_t types
#include <inttypes.h> // for PRIx8, etc.
#include "bit.h"
#include "ourError.h"


// ======================================================================

uint8_t lsb4(uint8_t value){
	return value & 0x0F;
}

// ======================================================================

uint8_t msb4(uint8_t value) {
	return (value & 0xF0)>>4;
}

// ======================================================================

uint8_t lsb8(uint16_t value) {
	return value & 0xFF;
}

// ======================================================================

uint8_t msb8(uint16_t value) {
	return (value & 0xFF00)>>8;
}

// ======================================================================

uint16_t merge8(uint8_t v1, uint8_t v2) {
	uint16_t result = v2;
	result = (result<<8) | v1;
	return result;
}

// ======================================================================

uint8_t merge4(uint8_t v1, uint8_t v2){
	return (lsb4(v2)<<4) | lsb4(v1);
}

// ======================================================================

bit_t bit_get(uint8_t value, int index){
	int index_clamped = CLAMP07(index);
    int mask = 1 << index_clamped;
    bit_t bit = (value & mask)>>index_clamped;
    return bit;
}

// ======================================================================

bit_t bit_get16(uint16_t value, int index){
    
    int index_clamped = CLAMP15(index);
    int mask = 1 << index_clamped;
    bit_t bit = (value & mask)>>index_clamped;
    return bit;
}

// ======================================================================

bit_t bit_get32(uint32_t value, int index){
    
    int index_clamped = CLAMP31(index);
    int mask = 1 << index_clamped;
    bit_t bit = (value & mask)>>index_clamped;
    return bit;
}

uint32_t bit_join32(uint32_t v1, uint32_t v2, int index){
    
    int index_clamped = CLAMP31(index);
    uint32_t maskLSB = (1 << index_clamped)-1;//Create a mask of 0...111
    uint32_t maskMSB = ~maskLSB;
    
    return (v1&maskLSB)| (v2&maskMSB);
}

// ======================================================================

void bit_set(uint8_t* value, int index){
	if(value == NULL){
		M_PRINT_NOT_NULL("value");
	}
	else {
		*value = *value | (1<<(CLAMP07(index)));
	}
}

// ======================================================================
void bit_unset(uint8_t* value, int index) {
	if(value == NULL){
		M_PRINT_NOT_NULL("value");
	}
	else {
		*value = *value & (~(1<<(CLAMP07(index))));
	}
}

// ======================================================================

void bit_rotate(uint8_t* value, rot_dir_t dir, int d){

	if(value == NULL){
		M_PRINT_NOT_NULL("value");
	}
	else {
		int index=CLAMP07(d);
		
		switch (dir) {
			case LEFT:{
				uint8_t mask= ((1<<index)-1)<<(8-index);
				uint8_t headCopy= (*value & mask)>>(8-index);
				*value= (*value<<index) | headCopy;
			}break;
				
				
			case RIGHT:{
				uint8_t mask= (1<<index)-1; // 1000 -1 = 0111
				uint8_t tailCopy= (*value & mask);
				*value=(*value>>index) | (tailCopy<<(8-index));
			  
			}break;
		}
	}
}
// ======================================================================

void bit_edit(uint8_t* value, int index, uint8_t v){
	
	if(value == NULL){
		M_PRINT_NOT_NULL("value");
	}
	else {
		index=CLAMP07(index);
		
		if(v==0){
			bit_unset(value,index);
		}

		else {
			bit_set(value,index);
		}
	}
}
