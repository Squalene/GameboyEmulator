#include <stdlib.h>
#include <stdio.h>
#include "cpu-registers.h"
#include "error.h"
#include "ourError.h"


uint16_t cpu_reg_pair_get(const cpu_t* cpu, reg_pair_kind reg){
	
	M_REQUIRE_NOT_NULL_RETURN(cpu, 0);
	switch (reg) {
		case REG_BC_CODE:
			return cpu->BC;
		break;
		
		case REG_DE_CODE:
			return cpu->DE;
		break;
		
		case REG_HL_CODE:
			return cpu->HL;
		break;
		
		case REG_AF_CODE:
			return cpu->AF;
		break;
		
		default:
			return 0;
	}
	

}
uint8_t cpu_reg_get(const cpu_t* cpu, reg_kind reg){
	M_REQUIRE_NOT_NULL_RETURN(cpu, 0);

	switch (reg) {
		case REG_B_CODE:
			return cpu->B;
		break;
		
		case REG_C_CODE:
			return cpu->C;
		break;
		
		case REG_D_CODE:
			return cpu->D;
		break;
		
		case REG_E_CODE:
			return cpu->E;
		break;

		case REG_H_CODE:
			return cpu->H;
		break;
		
		case REG_L_CODE:
			return cpu->L;
		break;
		
		case REG_A_CODE:
			return cpu->A;
		break;
		default:
			return 0;
	}
}

void cpu_reg_pair_set(cpu_t* cpu, reg_pair_kind reg, uint16_t value){
	if(cpu != NULL){
		switch (reg) {
			case REG_BC_CODE:
				cpu->BC = value;
			break;
			
			case REG_DE_CODE:
				cpu->DE = value;
			break;
			
			case REG_HL_CODE:
				cpu->HL = value;
			break;
			
			case REG_AF_CODE:
				cpu->AF = (value & 0xFFF0);
			break;		
			
		}
	}
	else {
		M_PRINT_NOT_NULL("cpu");
	}
}

void cpu_reg_set(cpu_t* cpu, reg_kind reg, uint8_t value){
	if(cpu != NULL){
		switch (reg) {
			case REG_B_CODE:
				cpu->B = value;
			break;
			
			case REG_C_CODE:
				cpu->C = value;
			break;
			
			case REG_D_CODE:
				cpu->D = value;
			break;
			
			case REG_E_CODE:
				cpu->E = value;
			break;

			case REG_H_CODE:
				cpu->H = value;
			break;
			
			case REG_L_CODE:
				cpu->L = value;
			break;
			
			case REG_A_CODE:
				cpu->A = value;
			break;
		}
	}
	else {
		M_PRINT_NOT_NULL("cpu");
	}
}
