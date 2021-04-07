/**
 * @file cpu-registers.c
 * @brief Game Boy CPU simulation, register part
 *
 * @date 2019
 */

#include "error.h"
#include "ourError.h"
#include "cpu-storage.h" // cpu_read_at_HL
#include "cpu-registers.h" // cpu_BC_get
#include "gameboy.h" // REGISTER_START
#include "util.h"
#include "memory.h" //data_t
#include "cpu.h"
#include "bus.h"
#include <inttypes.h> // PRIX8
#include <stdio.h> // fprintf

// ==== see cpu-storage.h ========================================
data_t cpu_read_at_idx(const cpu_t* cpu, addr_t addr){
    
    M_REQUIRE_NOT_NULL_RETURN(cpu, 0);
    M_REQUIRE_NOT_NULL_RETURN(cpu->bus, 0);
    
    data_t data=0;
    int err=bus_read(*(cpu->bus), addr, &data);

    if(err!=ERR_NONE){
        M_PRINT_ERROR(err);
        return 0;
    }
    
    return data;
}

// ==== see cpu-storage.h ========================================
addr_t cpu_read16_at_idx(const cpu_t* cpu, addr_t addr){
    
    M_REQUIRE_NOT_NULL_RETURN(cpu, 0);
    M_REQUIRE_NOT_NULL_RETURN(cpu->bus, 0);

    
    addr_t data=0;
    int err=bus_read16(*(cpu->bus), addr, &data);
    
    if(err!=ERR_NONE){
        M_PRINT_ERROR(err);
        return 0;
    }
    
 return data;
}

// ==== see cpu-storage.h ========================================
int cpu_write_at_idx(cpu_t* cpu, addr_t addr, data_t data){
    
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(cpu->bus);
    
    //Store the adress in the listener of the cpu
    cpu->write_listener = addr;
    
    return bus_write(*(cpu->bus), addr, data);
}

// ==== see cpu-storage.h ========================================
int cpu_write16_at_idx(cpu_t* cpu, addr_t addr, addr_t data16){

    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(cpu->bus);
    
    //Store the adress in the listener of the cpu
    cpu->write_listener = addr;
    
    return bus_write16(*(cpu->bus), addr, data16);
}

// ==== see cpu-storage.h ========================================
int cpu_SP_push(cpu_t* cpu, addr_t data16){
	
    M_REQUIRE_NON_NULL(cpu);
    cpu->SP -=sizeof(addr_t);
    
    return cpu_write16_at_idx(cpu,cpu->SP,data16);
}

// ==== see cpu-storage.h ========================================
addr_t cpu_SP_pop(cpu_t* cpu){
    
    M_REQUIRE_NOT_NULL_RETURN(cpu, 0);
    
    addr_t data=cpu_read16_at_idx(cpu, cpu->SP);
    
    cpu->SP +=sizeof(addr_t);
    
    return data;
}

// ==== see cpu-storage.h ========================================
int cpu_dispatch_storage(const instruction_t* lu, cpu_t* cpu){
    M_REQUIRE_NON_NULL(cpu);

    switch (lu->family) {
    
    //Load instruction
    case LD_A_BCR:{
        data_t value=cpu_read_at_idx(cpu, cpu_BC_get(cpu));
        cpu_reg_set(cpu, REG_A_CODE, value);
        }break;

    case LD_A_CR:{
        addr_t address= REGISTERS_START+cpu_reg_get(cpu, REG_C_CODE);
        data_t value=cpu_read_at_idx(cpu, address);
        cpu_reg_set(cpu, REG_A_CODE, value);
    }break;

    case LD_A_DER:{
        data_t value=cpu_read_at_idx(cpu, cpu_DE_get(cpu));
        cpu_reg_set(cpu, REG_A_CODE, value);
    }break;

    case LD_A_HLRU:{
        data_t value=cpu_read_at_HL(cpu);
        cpu_reg_set(cpu, REG_A_CODE, value);
        cpu_HL_set(cpu, cpu_HL_get(cpu) + extract_HL_increment(lu->opcode));
    }break;

    case LD_A_N16R:{
        addr_t address=cpu_read_addr_after_opcode(cpu);
        data_t value = cpu_read_at_idx(cpu, address);
        cpu_reg_set(cpu, REG_A_CODE, value);
    }break;

    case LD_A_N8R:{
        addr_t address=REGISTERS_START+cpu_read_data_after_opcode(cpu);
        data_t value = cpu_read_at_idx(cpu, address);
        cpu_reg_set(cpu, REG_A_CODE, value);
    }break;
            
    case LD_R16SP_N16:{
        reg_pair_kind regPairKind= extract_reg_pair(lu->opcode);
        addr_t value=cpu_read_addr_after_opcode(cpu);
        
        if(regPairKind == REG_AF_CODE){
			cpu->SP = value;
		}
		else{
			cpu_reg_pair_set(cpu, regPairKind, value);
		}
    }break;

    case LD_R8_HLR:{
        
        reg_kind regKind= extract_reg(lu->opcode, 3);
        addr_t value=cpu_read_at_HL(cpu);
        cpu_reg_set(cpu, regKind, value);
    } break;
   

    case LD_R8_N8:{
        
        reg_kind regKind= extract_reg(lu->opcode, 3);
        data_t value=cpu_read_data_after_opcode(cpu);
        cpu_reg_set(cpu, regKind, value);
    }break;
    
    case POP_R16:{
    
        reg_pair_kind regPairKind= extract_reg_pair(lu->opcode);
        addr_t value = cpu_SP_pop(cpu);
        cpu_reg_pair_set(cpu, regPairKind, value);
    }break;
    
            
            
    //Store instruction
    case LD_BCR_A:{
        data_t value= cpu_reg_get(cpu, REG_A_CODE);
        M_EXIT_IF_ERR(cpu_write_at_idx(cpu, cpu_BC_get(cpu), value));
        
    }break;

    case LD_CR_A:{
        
        data_t value= cpu_reg_get(cpu, REG_A_CODE);
        addr_t address=REGISTERS_START+cpu_reg_get(cpu, REG_C_CODE);
        M_EXIT_IF_ERR(cpu_write_at_idx(cpu, address, value));
        
    }break;
    

    case LD_DER_A:{
        data_t value= cpu_reg_get(cpu, REG_A_CODE);
        M_EXIT_IF_ERR(cpu_write_at_idx(cpu, cpu_DE_get(cpu), value));
    }break;
    

    case LD_HLRU_A:{
        
        data_t value=cpu_reg_get(cpu, REG_A_CODE);
        cpu_write_at_HL(cpu, value);
        cpu_HL_set(cpu, cpu_HL_get(cpu) + extract_HL_increment(lu->opcode));
        
    }break;
    

    case LD_HLR_N8:{
        
        data_t value=cpu_read_data_after_opcode(cpu);
        M_EXIT_IF_ERR(cpu_write_at_HL(cpu, value));
        
    }break;
    

    case LD_N16R_A:{
        
        addr_t address=cpu_read_addr_after_opcode(cpu);
        data_t value=cpu_reg_get(cpu, REG_A_CODE);
        M_EXIT_IF_ERR(cpu_write_at_idx(cpu, address, value));
    }break;
    

    case LD_N16R_SP:{
        addr_t address=cpu_read_addr_after_opcode(cpu);
        addr_t value=cpu->SP;
        M_EXIT_IF_ERR(cpu_write16_at_idx(cpu, address, value));
    }break;

    case LD_N8R_A:{
        addr_t address=REGISTERS_START+cpu_read_data_after_opcode(cpu);
        data_t value= cpu_reg_get(cpu, REG_A_CODE);
        M_EXIT_IF_ERR(cpu_write_at_idx(cpu, address, value));
        
    }break;
            
    case LD_HLR_R8:{
        
        reg_kind regKind=extract_reg(lu->opcode, 0);
        data_t value= cpu_reg_get(cpu, regKind);
        M_EXIT_IF_ERR(cpu_write_at_HL(cpu, value));
        
    }break;
    
   
    case PUSH_R16:{
        reg_pair_kind regPairKind= extract_reg_pair(lu->opcode);
        addr_t value =  cpu_reg_pair_get(cpu, regPairKind);
        M_EXIT_IF_ERR(cpu_SP_push(cpu, value));
        
    }break;
            
    //Copy instructions
    case LD_SP_HL:{
        cpu->SP=cpu_HL_get(cpu);
    }break;
            
    case LD_R8_R8: {
        
        reg_kind regKindFrom=extract_reg(lu->opcode, 0);
        reg_kind regKindTo=extract_reg(lu->opcode, 3);
        
        if(regKindFrom==regKindTo){
            fprintf(stderr, "Reg1==reg2, you should use NOP family instruction");
            return ERR_BAD_PARAMETER;
        }
        
        data_t value= cpu_reg_get(cpu, regKindFrom);
        cpu_reg_set(cpu, regKindTo, value);
    
    }break;

    default:
        fprintf(stderr, "Unknown STORAGE instruction, Code: 0x%" PRIX8 "\n", cpu_read_at_idx(cpu, cpu->PC));
        return ERR_INSTR;
        break;
    } // switch

    return ERR_NONE;
}
