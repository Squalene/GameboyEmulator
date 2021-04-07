#include <stdio.h>
#include "cartridge.h"
#include "error.h"
#include "ourError.h"
#include "memory.h"
#include "component.h"
#include "bus.h"

int cartridge_init_from_file(component_t* c, const char* filename){
	
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(c->mem);
	M_REQUIRE_NON_NULL(c->mem->memory);
	M_REQUIRE_NON_NULL(filename);

	FILE* input = fopen(filename, "rb");

	if(input == NULL) {
		M_PRINT_ERROR(ERR_IO);		
		return ERR_IO;
	}
    M_REQUIRE(c->mem->size >= sizeof(data_t)*BANK_ROM_SIZE,ERR_MEM,"Component memory (%lu) size is too small for the file",c->mem->size);
	size_t nb_ok = fread(c->mem->memory, sizeof(data_t), BANK_ROM_SIZE, input);
	M_REQUIRE(fclose(input)==0, ERR_IO, "Unable to close file %s", filename);

	M_REQUIRE(nb_ok == BANK_ROM_SIZE, ERR_IO, "Unable to read %zu elements, read only %zu", BANK_ROM_SIZE, nb_ok);
	M_REQUIRE(c->mem->memory[CARTRIDGE_TYPE_ADDR]==0, ERR_NOT_IMPLEMENTED, "Our gameboy only supports cartridge that only contains ROM of size %zu, nothing else.", BANK_ROM_SIZE);	
	
	return ERR_NONE;
}

int cartridge_init(cartridge_t* ct, const char* filename){

	M_REQUIRE_NON_NULL(ct);
	M_REQUIRE_NON_NULL(filename);

	M_EXIT_IF_ERR(component_create(&(ct->c), BANK_ROM_SIZE)); //Create the component (allocate memory)
	int err = cartridge_init_from_file(&(ct->c), filename); //Init its memory from the given filename
	
	//Free the component if we had an error
	if(err != ERR_NONE){
		component_free(&(ct->c));
		return err;
	}
	
	return ERR_NONE;
}

int cartridge_plug(cartridge_t* ct, bus_t bus){
	
	M_REQUIRE_NON_NULL(ct);
	M_REQUIRE_NON_NULL(bus);
	
	M_EXIT_IF_ERR(bus_forced_plug(bus, &(ct->c), BANK_ROM0_START, BANK_ROM1_END, 0));

	return ERR_NONE;
}

void cartridge_free(cartridge_t* ct){
	if(ct != NULL){
		component_free(&(ct->c));
	}
}


