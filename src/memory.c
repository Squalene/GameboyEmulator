#include <stdlib.h>
#include "memory.h"
#include "error.h"
#include "util.h"

int mem_create(memory_t* mem, size_t size){
	
	M_REQUIRE_NON_NULL(mem);
	M_REQUIRE(size != 0, ERR_BAD_PARAMETER, "Size (%lu) is 0", size);
	
	zero_init_ptr(mem);

	M_EXIT_IF_NULL(mem->memory = calloc(size, sizeof(data_t)), size*sizeof(data_t));
	
	mem->size = size;
	
	return ERR_NONE;
}

void mem_free(memory_t* mem) {
	if(mem != NULL){
		free(mem->memory);
		mem->memory = NULL;
		mem->size = 0;
	}
}
