#include <stdio.h>
#include <stdint.h> //uint8_t

#include "alu.h"
#include "error.h"
#include "ourError.h"
#include "bit.h"

#define BYTE_SIZE 8



flags_t add16_compute_flag(uint16_t value, uint8_t x,uint8_t y, bit_t carry);
bit_t has_flag(flags_t flags, flag_bit_t flag);
void alu_assignFlags(alu_output_t* result,uint8_t msbs, uint8_t lsbs);
void alu_assignShiftRotateFlags(alu_output_t* result, bit_t carry);

/**
 * @brief If result is zero, set flag Z, otherwise unset it.
 */
#define CHOOSE_Z(result)\
	if(result->value == 0){\
		set_Z(&(result->flags));\
	}\
	else {\
		unset_Z(&(result->flags));\
	}

flag_bit_t get_flag(flags_t flags, flag_bit_t flag){
    
    switch (flag) {
        case FLAG_Z:
        case FLAG_N:
        case FLAG_H:
        case FLAG_C:{
            return (flags|flag)==flags?flag:0;
        }break;
        default:{
            return 0;
        }break;
    }
}

/**
 * @brief Check whether a given flag is set or not in a flags_t
 *
 * @param flags: the flags to check
 * @param flag: the given flag
 */
bit_t has_flag(flags_t flags, flag_bit_t flag){
    
    return get_flag(flags,flag)!=0?1:0;
}

void set_flag(flags_t* flags, flag_bit_t flag){
	
	if(flags == NULL){
		M_PRINT_NOT_NULL("flags");
	}
	else {
		switch (flag) {
			case FLAG_C:
			case FLAG_H:
			case FLAG_N:
			case FLAG_Z:{
				*flags = *flags | flag;
			}break;
				
			default:break;//nothing to be done
		}
	}
}
/**
 * @brief Unset a flag (opposite as set_flag).
 *
 * @param flags: the flags that we must modify
 * @param flag: the flag to unset
 */
void unset_flag(flags_t* flags, flag_bit_t flag){
	
	if(flags == NULL){
		M_PRINT_NOT_NULL("flags");
	}
	else {	
		switch (flag) {
			case FLAG_C:
			case FLAG_H:
			case FLAG_N:
			case FLAG_Z:{
				*flags = *flags & (~flag);
			}
			break;
			  
			default:break;//nothing to be done
		   }
	}
}

int alu_add8(alu_output_t* result, uint8_t x, uint8_t y, bit_t c0){
	
	M_REQUIRE_NON_NULL(result);
		
	uint8_t lsbs = lsb4(x) + lsb4(y) + c0;
	uint8_t msbs = msb4(x) + msb4(y) + msb4(lsbs);
	result->value = merge4(lsbs, msbs);
	
	
	result->flags=0;
	alu_assignFlags(result,msbs,lsbs);
	
	return ERR_NONE;


}


int alu_sub8(alu_output_t* result, uint8_t x, uint8_t y, bit_t b0){
	
	M_REQUIRE_NON_NULL(result);
	
		
	uint8_t lsbs = lsb4(x) - lsb4(y) - b0;
	uint8_t msbs = msb4(x) - msb4(y) - (msb4(lsbs)==0?0:1);
	result->value = merge4(lsbs, msbs);
	
	
	result->flags = FLAG_N; //N should always be set to 1
	alu_assignFlags(result,msbs,lsbs);
	
	return ERR_NONE;
}



int alu_add16_low(alu_output_t* result, uint16_t x, uint16_t y){
	
	M_REQUIRE_NON_NULL(result);
		
	alu_add8(result, lsb8(x), lsb8(y),0);
	uint16_t msbs = msb8(x) + msb8(y) + (get_flag(result->flags, FLAG_C) == FLAG_C? 1:0);
	result->value = merge8(result->value, msbs); 
	
	CHOOSE_Z(result);

	return ERR_NONE;
}

int alu_add16_high(alu_output_t* result, uint16_t x, uint16_t y){
	
	M_REQUIRE_NON_NULL(result);
		
	uint16_t lsbs = lsb8(x) + lsb8(y);
	M_EXIT_IF_ERR(alu_add8(result, msb8(x), msb8(y), bit_get(msb8(lsbs), 0)));
	result->value = merge8(lsbs, result->value);
	
	CHOOSE_Z(result);

	return ERR_NONE;
}

//logical shift
int alu_shift(alu_output_t* result, uint8_t x,rot_dir_t dir){
    
    M_REQUIRE_NON_NULL(result);
    
    bit_t erasedBit=0;
    switch (dir) {
            
        case LEFT:{
            erasedBit=bit_get(x, BYTE_SIZE-1);
            x=x<<1;
        }break;
            
        case RIGHT:{
            erasedBit=bit_get(x, 0);
            x=x>>1;
        }break;
        default:
            return ERR_BAD_PARAMETER;
        break;
    }
    
    result->value = x;
    alu_assignShiftRotateFlags(result,erasedBit);
    
    return ERR_NONE;
}

int alu_shiftR_A(alu_output_t* result, uint8_t x){
    
    M_REQUIRE_NON_NULL(result);
    
    bit_t erasedBit=bit_get(x, 0);
    bit_t signBit=bit_get(x, BYTE_SIZE-1);
    
    //rotate of 1 to the right and take into account sign bit
    result->value=(signBit<<(BYTE_SIZE-1)| x>>1);
    alu_assignShiftRotateFlags(result,erasedBit);
    
    return ERR_NONE;
}

int alu_rotate(alu_output_t* result, uint8_t x, rot_dir_t dir){
    
    M_REQUIRE_NON_NULL(result);
    
    bit_t rotatedBit=0;
    switch (dir) {
            
        case LEFT:{
            rotatedBit=bit_get(x, BYTE_SIZE-1);
        }break;
            
        case RIGHT:{
            rotatedBit=bit_get(x, 0);
        }break;
        
        default:
            return ERR_BAD_PARAMETER;
        break;
    }
    
    bit_rotate(&x, dir, 1);
    
    result->value=x;//arithmetic by default
    alu_assignShiftRotateFlags(result,rotatedBit);
    
    return ERR_NONE;
}

int alu_carry_rotate(alu_output_t* result, uint8_t x, rot_dir_t dir, flags_t flags){
	
	M_REQUIRE_NON_NULL(result);

	bit_t keptValue = 0;
	bit_t c = has_C(flags);
	
	switch (dir) {
		case LEFT:{
			keptValue = bit_get(x, BYTE_SIZE-1) ;
			x = x<<1;
			result->value = x | c;
		}break;   
		case RIGHT:{
			keptValue = bit_get(x, 0) ;
			x = x>>1;
			result->value = x | (c<<(BYTE_SIZE-1));
		}break;
		
		default:
            return ERR_BAD_PARAMETER;
        break;
	}
    
    alu_assignShiftRotateFlags(result,keptValue);
	
	return ERR_NONE;   
}

/**
 * @brief Assign correct flags to alu after addition/substraction.
 *
 * @param result: alu_output in which we must assign the flags
 * @param msbs: msbs of the operation
 * @param lsbs: lsbs of the operation
 */
void alu_assignFlags(alu_output_t* result,uint8_t msbs, uint8_t lsbs){
    
    if(result == NULL){
        M_PRINT_NOT_NULL("result");
    }
    else {
        if (result->value == 0){
            set_Z(&(result->flags));
        }
        if (msb4(lsbs)!=0){
            set_H(&(result->flags));
        }
        if (msb4(msbs)!=0){
            set_C(&(result->flags));
        }
    }
}

/**
* @brief Assign correct flags to alu after shift or rotate
*
* @param result: alu_output  in which we must assign the flags
* @param carry: bit used to determine the C flag
*
*/
void alu_assignShiftRotateFlags(alu_output_t* result, bit_t carry){
    
    if(result == NULL){
        M_PRINT_NOT_NULL("result");
    }
    else {
        
        result->flags = result->value == 0 ? FLAG_Z : 0;
        
        if(carry==1){
           set_C(&(result->flags));
        }
    }
}
