/**
 * @file cpu.c
 * @brief Game Boy CPU simulation
 *
 * @date 2019
 */

#include "error.h"
#include "opcode.h"
#include "cpu.h"
#include "cpu-alu.h"
#include "cpu-registers.h"
#include "cpu-storage.h"
#include "util.h"
#include "ourError.h"
#include "bit.h"
#include "component.h"
#include "memory.h"
#include "bus.h"

#include <inttypes.h> // PRIX8
#include <stdio.h> // fprintf




bit_t cpu_check_CC (flags_t flags, opcode_t opcode);
int call_instructions(cpu_t* cpu, const instruction_t* lu, addr_t address);
bit_t findInterrupt(cpu_t* cpu, interrupt_t* interrupt);
int jr_e8_instructions(cpu_t* cpu, const instruction_t* lu);
static int cpu_do_cycle(cpu_t* cpu);

#define GET_INTERRUPT_ADDRESS(interrupt)\
	0x40 + (interrupt << 3)

#define GET_INTERRUPTS(cpu)\
    (cpu->IE & cpu->IF) & INTERRUPT_MASK //We only use the first 5 bits to indicate interrupts

#define INTERRUPTION_CYCLES 5

#define INTERRUPT_MASK 31 //0x1F

// ======================================================================
int cpu_init(cpu_t* cpu){
    
    M_REQUIRE_NON_NULL(cpu);
    
    zero_init_ptr(cpu);
    
    M_EXIT_IF_ERR(component_create(&cpu->high_ram, HIGH_RAM_SIZE));
    
    return ERR_NONE;
}

// ======================================================================
int cpu_plug(cpu_t* cpu, bus_t* bus)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(bus);
    
    cpu->bus=bus;
    
    M_EXIT_IF_ERR(bus_plug(*(cpu->bus), &cpu->high_ram, HIGH_RAM_START, HIGH_RAM_END));
    
    (*bus)[REG_IE] = &cpu->IE;
    (*bus)[REG_IF] = &cpu->IF;
    
    return ERR_NONE;
}

// ======================================================================
void cpu_free(cpu_t* cpu){
    if(cpu!=NULL){
        M_PRINT_IF_ERROR(bus_unplug(*(cpu->bus), &(cpu->high_ram)), "Cpu highRam could not be unplugged");

        component_free(&(cpu->high_ram));
		cpu->IE=0;
		cpu->IF=0;
        if(cpu->bus!=NULL){
            (*cpu->bus)[REG_IE] = NULL;
            (*cpu->bus)[REG_IF] =NULL;
            cpu->bus=NULL;
        }
    }
}

//=========================================================================
/**
 * @brief Executes an instruction
 * @param lu instruction
 * @param cpu, the CPU which shall execute
 * @return error code
 *
 * See opcode.h and cpu.h
 */
static int cpu_dispatch(const instruction_t* lu, cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(lu);
    M_REQUIRE_NON_NULL(cpu);
    
    zero_init_var(cpu->alu);//Reset flags and value to 0
        
    cpu->idle_time=lu->cycles-1;
    
    switch (lu->family) {

    // ALU
    case ADD_A_HLR:
    case ADD_A_N8:
    case ADD_A_R8:
    case INC_HLR:
    case INC_R8:
    case ADD_HL_R16SP:
    case INC_R16SP:
    case SUB_A_HLR:
    case SUB_A_N8:
    case SUB_A_R8:
    case DEC_HLR:
    case DEC_R8:
    case DEC_R16SP:
    case AND_A_HLR:
    case AND_A_N8:
    case AND_A_R8:
    case OR_A_HLR:
    case OR_A_N8:
    case OR_A_R8:
    case XOR_A_HLR:
    case XOR_A_N8:
    case XOR_A_R8:
    case CPL:
    case CP_A_HLR:
    case CP_A_N8:
    case CP_A_R8:
    case SLA_HLR:
    case SLA_R8:
    case SRA_HLR:
    case SRA_R8:
    case SRL_HLR:
    case SRL_R8:
    case ROTCA:
    case ROTA:
    case ROTC_HLR:
    case ROT_HLR:
    case ROTC_R8:
    case ROT_R8:
    case SWAP_HLR:
    case SWAP_R8:
    case BIT_U3_HLR:
    case BIT_U3_R8:
    case CHG_U3_HLR:
    case CHG_U3_R8:
    case LD_HLSP_S8:
    case DAA:
    case SCCF:{
        M_EXIT_IF_ERR(cpu_dispatch_alu(lu, cpu));
        cpu->PC+=lu->bytes;//increment PC counter
    } break;
       

    // STORAGE
    case LD_A_BCR:
    case LD_A_CR:
    case LD_A_DER:
    case LD_A_HLRU:
    case LD_A_N16R:
    case LD_A_N8R:
    case LD_BCR_A:
    case LD_CR_A:
    case LD_DER_A:
    case LD_HLRU_A:
    case LD_HLR_N8:
    case LD_HLR_R8:
    case LD_N16R_A:
    case LD_N16R_SP:
    case LD_N8R_A:
    case LD_R16SP_N16:
    case LD_R8_HLR:
    case LD_R8_N8:
    case LD_R8_R8:
    case LD_SP_HL:
    case POP_R16:
    case PUSH_R16:{
        M_EXIT_IF_ERR(cpu_dispatch_storage(lu, cpu));
		cpu->PC+=lu->bytes;//increment PC counter
    }break;


    // JUMP
            
    case JP_N16:{
        addr_t address = cpu_read_addr_after_opcode(cpu);
        cpu->PC =address;
    }break;
            
    case JP_CC_N16:{
        
        if(cpu_check_CC(cpu->F, lu->opcode)){
            addr_t address = cpu_read_addr_after_opcode(cpu);
            cpu->PC=address;
            cpu->idle_time+= lu->xtra_cycles;
        }
        else{
			cpu->PC += lu->bytes;
		}
    }break;

    case JP_HL:{
        cpu->PC = cpu_HL_get(cpu);
    }break;

    case JR_E8:{
        M_EXIT_IF_ERR(jr_e8_instructions(cpu, lu));
        
    }break;

    case JR_CC_E8:{
         if(cpu_check_CC(cpu->F, lu->opcode)){
            M_EXIT_IF_ERR(jr_e8_instructions(cpu, lu));
            cpu->idle_time+= lu->xtra_cycles;
         }
         else{
			cpu->PC += lu->bytes;
		}
    }break;

    // CALLS
    case CALL_N16:{
		addr_t address = cpu_read_addr_after_opcode(cpu);
		M_EXIT_IF_ERR(call_instructions(cpu, lu, address));
    }break;
    
    case CALL_CC_N16:{
		 if(cpu_check_CC(cpu->F, lu->opcode)){
			 	 
			addr_t address = cpu_read_addr_after_opcode(cpu);
			M_EXIT_IF_ERR(call_instructions(cpu, lu, address));
			
			cpu->idle_time += lu->xtra_cycles;
         }
         else{
			cpu->PC += lu->bytes;
		}
    }break;



    // RETURN (from call)
    case RST_U3:{
		addr_t address = 8 * extract_n3(lu->opcode);
		M_EXIT_IF_ERR(call_instructions(cpu, lu, address));

    }break;
    
    case RET:{
		cpu->PC = cpu_SP_pop(cpu);
	}break;

    case RET_CC:{
		if(cpu_check_CC(cpu->F, lu->opcode)){
			cpu->PC = cpu_SP_pop(cpu);
			cpu->idle_time += lu->xtra_cycles;
		}
		else{
			cpu->PC += lu->bytes;
		}
    }break;




    // INTERRUPT & MISC.
    case EDI:{
		cpu->IME = extract_ime(lu->opcode);
		cpu->PC+=lu->bytes;//increment PC counter

	}break;

    case RETI:{
		cpu->IME = 1; //Enable IME to accept interrupts again
		cpu->PC = cpu_SP_pop(cpu);
    }break;

    case HALT:{
        
        cpu->HALT=1;
        cpu->PC += lu->bytes;//increment PC counter

    }break;

    case STOP:
    case NOP:
        // ne rien faire
        cpu->PC+=lu->bytes;//increment PC counter
        break;

    default: {
        fprintf(stderr, "Unknown instruction, Code: 0x%" PRIX8 "\n", cpu_read_at_idx(cpu, cpu->PC));
        return ERR_INSTR;
    } break;

    } // switch
        
    return ERR_NONE;
}

// ----------------------------------------------------------------------

/**
* @brief Check for interrupts and redirects PC to deal with them, if no interrupts, execute next instruction
*
* @param cpu cpu to start
*
* @return error code
*/
static int cpu_do_cycle(cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(cpu);
    
    interrupt_t interrupt = 0;
    if(cpu->IME && findInterrupt(cpu, &interrupt)){
        
        cpu->IME=0;
        bit_unset(&cpu->IF,interrupt);
        M_EXIT_IF_ERR(cpu_SP_push(cpu, cpu->PC));
        cpu->PC = GET_INTERRUPT_ADDRESS(interrupt); //Go to the corresponding interrupt handler
        cpu->idle_time+=INTERRUPTION_CYCLES;
    }
    
    else {
    
        opcode_t opcode = cpu_read_at_idx(cpu, cpu->PC);//Gets next opcode instruction from memory pointed by PC
        
        instruction_t instruction;
        zero_init_var(instruction);

        if(opcode == PREFIXED) {
            opcode_t opcode2 = cpu_read_data_after_opcode(cpu);
            instruction = instruction_prefixed[opcode2];//Convert opcode to instruction
        }
        else{
            instruction = instruction_direct[opcode];//Convert opcode to instruction
        }
        M_EXIT_IF_ERR(cpu_dispatch(&instruction, cpu));//Execute the instruction
    }
    return ERR_NONE;
    
}

// ======================================================================
/**
 * See cpu.h
 */
int cpu_cycle(cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(cpu->bus);
    
    //Initialize write_listener to 0
    cpu->write_listener = 0;
    
    if(cpu->idle_time!=0){
        --cpu->idle_time;
    }
    
    else {
        //if halted and there is an interrupt => we wake up the cpu
        if(cpu->HALT && GET_INTERRUPTS(cpu)){
            cpu->HALT=0;
        }
        
        if(!cpu->HALT){
         M_EXIT_IF_ERR(cpu_do_cycle(cpu));
        }
    }
    
    return ERR_NONE;
}

void cpu_request_interrupt(cpu_t* cpu, interrupt_t i){
	
	if(cpu == NULL){
		M_PRINT_NOT_NULL("cpu");
	}
	else{
		bit_set(&cpu->IF, i);
	}
}



/**
 * @brief Check if a condition for a branching is true;
 *
 * @param flags: the flags (usually the register containing the flags, i.e. F)
 * @param opcode: the opcode of the instruction
 *
 * @return 1 if the condition is tested, 0 otherwise.
 */
bit_t cpu_check_CC (flags_t flags, opcode_t opcode){
    
    data_t cc = extract_cc(opcode);
    switch (cc) {
        case 0:{ //0b00
            return !has_Z(flags);
        }break;
        
        case 1:{ //0b01
            return has_Z(flags);
        }break;
        
        case 2:{ //0b10
             return !has_C(flags);
        }break;
        
        case 3:{ //0b11
              return has_C(flags);
        }break;
            
        default:{
            return 0;
        }break;
    }
}

/**
 * @brief Common code for all of the call instructions;
 *
 * @param cpu: the cpu
 * @param lu: the instruction
 * @param nextAddr: the next address where we need to jump
 *
 * @return the error or err_none if no error occured
 */
int call_instructions(cpu_t* cpu, const instruction_t* lu, addr_t calledAddress){
	
	addr_t nextInstruction = cpu->PC + lu->bytes;
    M_EXIT_IF_ERR(cpu_SP_push(cpu, nextInstruction)); //Push next instruction in SP
	
	cpu->PC = calledAddress;
	
	return ERR_NONE;
}


/**
 * @brief Find the first interrupt that is active and return 1 if it was found or 0 otherwise;
 *
 * @param cpu: the cpu
 * @param interrupt: a pointer to the interrupt that was found
 *
 * @return 1 if we found an interrupt, 0 otherwise.
 */
bit_t findInterrupt(cpu_t* cpu, interrupt_t* interrupt){
    data_t interrupts =GET_INTERRUPTS(cpu);
    interrupt_t counter = VBLANK; //First Interrupt
    bit_t found = 0;
    //Find the first bit which is 1
    while(!found && counter<=JOYPAD){
        if(bit_get(interrupts, counter)){
            found = 1;
        }
        else{
            ++counter;
        }
    }
    
    if(found){
        *interrupt = counter;
    }
    
    return found;
}

/**
 * @brief Common part of jr_e8 instructions;
 *
 * @param cpu: the cpu
 * @param lu: the instruction
 *
 * @return ERR_BAD_PARAMETER if the value of the PC would have become negative
 * 		   or ERR_NONE if everything was fine.
 */
int jr_e8_instructions(cpu_t* cpu, const instruction_t* lu){
	
	// Extend to its 16 bit representation by extending its sign bit
	int16_t increment =  extend_s_16(cpu_read_data_after_opcode(cpu));
	int32_t incrementedPC = cpu->PC + increment+ lu->bytes; //We use 32 bits in case where cpu-PC start with 1 (we do not want an error in this case)
	
	if(incrementedPC < 0){
		return ERR_BAD_PARAMETER;
	}

	cpu->PC = incrementedPC;//increment PC counter by increment starting from next instruction
	
	return ERR_NONE;
}
