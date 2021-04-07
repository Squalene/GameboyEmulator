#include <stdlib.h>
#include "component.h"
#include "error.h"
#include "memory.h"

int component_create(component_t* c, size_t mem_size){
    
    M_REQUIRE_NON_NULL(c);
		
	c->start=0;
	c->end=0;
	
	if(mem_size == 0){
		//Component doesn't have memory
		c->mem = NULL;
		return ERR_NONE;
	}
	else{
		M_EXIT_IF_NULL(c->mem = malloc(sizeof(memory_t)), sizeof(memory_t));

		int err = mem_create(c->mem, mem_size);
		if (err != ERR_NONE){
			component_free(c);
		}
		return err;
	}        
}

int component_shared(component_t* c, component_t* c_old) {
	
		M_REQUIRE_NON_NULL(c);
		M_REQUIRE_NON_NULL(c_old);
        M_REQUIRE_NON_NULL(c_old->mem);
        M_REQUIRE_NON_NULL(c_old->mem->memory);
		
		c->start = 0;
		c->end = 0;
		
		c->mem = c_old->mem;
		
		return ERR_NONE;
}



void component_free(component_t* c){
    if(c!=NULL){
        c->start=0;
        c->end=0;
        mem_free(c->mem);
        free(c->mem);
        c->mem = NULL;
    }
}
