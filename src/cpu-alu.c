/**
 * @file cpu-alu.c
 * @brief Game Boy CPU simulation, ALU part asked to students
 *
 * @date 2019
 */
#include "error.h"
#include "bit.h"
#include "alu.h"
#include "cpu-alu.h"
#include "cpu-storage.h" // cpu_read_at_HL
#include "cpu-registers.h" // cpu_HL_get
#include "opcode.h" //instruction_t
#include "memory.h" //data_t

// external library provided later to lower workload
extern int cpu_dispatch_alu_ext(const instruction_t* lu, cpu_t* cpu);

#include <assert.h>
#include <stdbool.h>

// ======================================================================
/**
 * @brief Returns flag bit value based on source preferences
 *
 * @param src source to select
 * @param cpu_f flags from cpu
 * @param alu_f flags from alu
 *
 * @return resulting flag bit value
 */
static bool flags_src_value(flag_src_t src, flag_bit_t cpu_f, flag_bit_t alu_f)
{
    switch (src) {
    case CLEAR:
        return false;

    case SET:
        return true;

    case ALU:
        return alu_f;

    case CPU:
        return cpu_f;

    default:
        return false;
    }

    return false;
}

// ======================================================================
/**
* @brief Checks if x is a valid flag source
*/
#define IS_VALID_FLAG_SRC(x) \
    (x == CLEAR || x == SET || x == ALU || x == CPU)
#define CHECK_FLAG_SRC(x) \
    M_REQUIRE(IS_VALID_FLAG_SRC(x), ERR_BAD_PARAMETER, "Parameter %d for " #x " is not valid", x)

// ==== see cpu-alu.h ========================================
int cpu_combine_alu_flags(cpu_t* cpu,
                          flag_src_t Z, flag_src_t N, flag_src_t H, flag_src_t C)
{
    M_REQUIRE_NON_NULL(cpu);
    CHECK_FLAG_SRC(Z);
    CHECK_FLAG_SRC(N);
    CHECK_FLAG_SRC(H);
    CHECK_FLAG_SRC(C);

    flags_t res_f = 0;

    if (flags_src_value(Z, get_Z(cpu->F), get_Z(cpu->alu.flags)))
        set_Z(&res_f);

    if (flags_src_value(N, get_N(cpu->F), get_N(cpu->alu.flags)))
        set_N(&res_f);

    if (flags_src_value(H, get_H(cpu->F), get_H(cpu->alu.flags)))
        set_H(&res_f);

    if (flags_src_value(C, get_C(cpu->F), get_C(cpu->alu.flags)))
        set_C(&res_f);

    cpu->F = res_f;

    return ERR_NONE;
}

// ======================================================================
/**
* @brief Tool function usefull for CHG_U3_R8:
*        Do a SET or a RESET(=unset) of data bit,
*          according to SR and N3 bits of instruction's opcode
*/
static void do_set_or_res(const instruction_t* lu, data_t* data)
{
    assert(lu   != NULL);
    assert(data != NULL);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

    if (extract_sr_bit(lu->opcode)) {
        *data |=   (data_t) (1 << extract_n3(lu->opcode)) ;
    } else {
        *data &= ~((data_t) (1 << extract_n3(lu->opcode)));
    }

#pragma GCC diagnostic pop
}

// ==== see cpu-alu.h ========================================
int cpu_dispatch_alu(const instruction_t* lu, cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(cpu);

    switch (lu->family) {

    // ADD
    case ADD_A_HLR: {
		
		addr_t hlValue=cpu_read_at_HL(cpu);
		do_cpu_arithm(cpu, alu_add8, hlValue, ADD_FLAGS_SRC);
		
    } break;

    case ADD_A_N8: {
		
		data_t value=cpu_read_data_after_opcode(cpu);
		do_cpu_arithm(cpu, alu_add8, value, ADD_FLAGS_SRC);

    } break;

    case ADD_A_R8: {
		
		reg_kind regKind = extract_reg(lu->opcode, 0);
		data_t value = cpu_reg_get(cpu, regKind);
		do_cpu_arithm(cpu, alu_add8, value, ADD_FLAGS_SRC);
		
    } break;

    case INC_HLR: {

    	addr_t hlValue=cpu_read_at_HL(cpu);
    	
		M_EXIT_IF_ERR(alu_add8(&cpu->alu, hlValue, 1, 0));
		M_EXIT_IF_ERR(cpu_combine_alu_flags(cpu, INC_FLAGS_SRC));
		
		M_EXIT_IF_ERR(cpu_write_at_HL(cpu, cpu->alu.value));

    } break;

    case INC_R8: {

		reg_kind regKind = extract_reg(lu->opcode, 3);
		data_t value = cpu_reg_get(cpu, regKind);
		    	
		M_EXIT_IF_ERR(alu_add8(&cpu->alu, value, 1, 0));
		M_EXIT_IF_ERR(cpu_combine_alu_flags(cpu, INC_FLAGS_SRC));
		
		cpu_reg_set_from_alu8(cpu, regKind);
		
    } break;

    case ADD_HL_R16SP: {
		
		addr_t hlValue=cpu_HL_get(cpu);
		
		reg_pair_kind regPairKind = extract_reg_pair(lu->opcode);
		
		// If the registerPair is AF (11), then we use SP instead of AF
		addr_t value = (regPairKind == REG_AF_CODE) ? cpu->SP : cpu_reg_pair_get(cpu, regPairKind);
    	
		M_EXIT_IF_ERR(alu_add16_high(&cpu->alu, hlValue, value));
		M_EXIT_IF_ERR(cpu_combine_alu_flags(cpu, ADD_NON_ZERO_FLAGS_SRC));
		
		cpu_HL_set(cpu, cpu->alu.value);
		
    } break;

    case INC_R16SP: {
		
		reg_pair_kind regPairKind = extract_reg_pair(lu->opcode);

		// If the registerPair is AF (11), then we use SP instead of AF
		addr_t value = (regPairKind == REG_AF_CODE) ? cpu->SP : cpu_reg_pair_get(cpu, regPairKind);
				    	
		M_EXIT_IF_ERR(alu_add16_low(&cpu->alu, value, 1)); //Don't care if low or high => don't use the flags		
		
		if(regPairKind == REG_AF_CODE){
			cpu->SP = cpu->alu.value;
		}
		else {
			cpu_reg_pair_set(cpu, regPairKind, cpu->alu.value);
		}
		
    } break;
    
    case DEC_R8: {
		reg_kind regKind = extract_reg(lu->opcode, 3);
		data_t value = cpu_reg_get(cpu, regKind);
		    	
		M_EXIT_IF_ERR(alu_sub8(&cpu->alu, value, 1, 0));
		M_EXIT_IF_ERR(cpu_combine_alu_flags(cpu, DEC_FLAGS_SRC));
		
		cpu_reg_set_from_alu8(cpu, regKind);
	} break;
	
	 // COMPARISONS
	case CP_A_N8: {
		data_t aValue = cpu_reg_get(cpu, REG_A_CODE);
		data_t opValue = cpu_read_data_after_opcode(cpu);

		M_EXIT_IF_ERR(alu_sub8(&cpu->alu, aValue, opValue, 0));
		M_EXIT_IF_ERR(cpu_combine_alu_flags(cpu, SUB_FLAGS_SRC));
	} break;


    case CP_A_R8: {
		
		data_t aValue = cpu_reg_get(cpu, REG_A_CODE);
		
		reg_kind regKind = extract_reg(lu->opcode, 0);
		data_t regValue = cpu_reg_get(cpu, regKind);

		M_EXIT_IF_ERR(alu_sub8(&cpu->alu, aValue, regValue, 0));
		M_EXIT_IF_ERR(cpu_combine_alu_flags(cpu, SUB_FLAGS_SRC));		
		
    } break;


    // BIT MOVE (rotate, shift)
    case SLA_R8: {
		
						
		reg_kind regKind = extract_reg(lu->opcode, 0);
		data_t regValue = cpu_reg_get(cpu, regKind);
		
		
		M_EXIT_IF_ERR(alu_shift(&cpu->alu, regValue, LEFT));
		M_EXIT_IF_ERR(cpu_combine_alu_flags(cpu, SHIFT_FLAGS_SRC));
		
		cpu_reg_set_from_alu8(cpu, regKind);

		
    } break;

    case ROT_R8: {
		
		reg_kind regKind = extract_reg(lu->opcode, 0);
		data_t regValue = cpu_reg_get(cpu, regKind);
		
		M_EXIT_IF_ERR(alu_carry_rotate(&cpu->alu, regValue, extract_rot_dir(lu->opcode), get_C(cpu->F)));
		M_EXIT_IF_ERR(cpu_combine_alu_flags(cpu, SHIFT_FLAGS_SRC));
		
		cpu_reg_set_from_alu8(cpu, regKind);
		
    } break;


    // BIT TESTS (and set)
    case BIT_U3_R8: {
		
		reg_kind regKind = extract_reg(lu->opcode, 0);
		data_t regValue = cpu_reg_get(cpu, regKind);
		
		data_t n3 = extract_n3(lu->opcode);
		
		
		M_EXIT_IF_ERR(cpu_combine_alu_flags(cpu, BIT_FLAGS_SRC));
		
		if(!bit_get(regValue, n3)){
			set_Z(&(cpu->F));
		}
		
    } break;

    case CHG_U3_R8: {
		
		reg_kind regKind = extract_reg(lu->opcode, 0);
		data_t regValue = cpu_reg_get(cpu, regKind);
		
		do_set_or_res(lu, &regValue);
		
		cpu_reg_set(cpu, regKind, regValue);

		
    } break;

    // ---------------------------------------------------------
    // All the others are handled elsewhere by provided library
    default:
        // uncomment this line if you have the cs212gbcpuext library
        M_EXIT_IF_ERR(cpu_dispatch_alu_ext(lu, cpu));
        break;
    } // switch

    return ERR_NONE;
}
