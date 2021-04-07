#include <stdio.h>
#include "bus.h"
#include "error.h"
#include "bit.h"
#include "component.h"
#include "memory.h"


#define DEFAULT_DATA 0xFF


int bus_remap(bus_t bus, component_t* c, addr_t offset){
    
    M_REQUIRE_NON_NULL(c);
    M_REQUIRE_NON_NULL(bus);
    M_REQUIRE_NON_NULL(c->mem);
    M_REQUIRE_NON_NULL(c->mem->memory);

    
    //If component is plugged
     if(!(c->start==0 && c->end==0)){
    
        uint16_t componentAllocatedSize= c->end-c->start;
        M_REQUIRE(componentAllocatedSize+offset < c->mem->size, ERR_ADDRESS, "Size (%lu) is bigger than c->mem->size", componentAllocatedSize+offset);

         //Modifies bus pointing address in range start->end to component range : offset-> offset+start-end
        for(size_t i=0;i<=componentAllocatedSize;++i){
			
            bus[c->start+i]= &(c->mem->memory[offset+i]);
        }
     }
    return ERR_NONE;
}

int bus_forced_plug(bus_t bus, component_t* c, addr_t start, addr_t end, addr_t offset){
    M_REQUIRE_NON_NULL(c);
    M_REQUIRE_NON_NULL(bus);
	
	//In case a requirement is not satisfied, is initialized to 0
    c->start=0;
    c->end=0; 
       
    M_REQUIRE(start<=end, ERR_ADDRESS, "Start (%lu) is bigger than end (%lu)", start, end);
    M_REQUIRE_NON_NULL(c->mem);
    M_REQUIRE(end-start<c->mem->size, ERR_ADDRESS, "Memory of component is not big enough for the range end-start (%lu)", end-start);
	
	c->start=start;
	c->end=end;
	
	//Connects the component to the bus in range start->end and with component range : offset-> start-end
	int err=bus_remap(bus, c, offset);

	if(err!=ERR_NONE){
		c->start=0;
		c->end=0;
		return err;
	}
	return ERR_NONE;
    
}

int bus_plug(bus_t bus, component_t* c, addr_t start, addr_t end){
    
    M_REQUIRE_NON_NULL(c);
    M_REQUIRE_NON_NULL(bus);
    
    //Verifies that the adress range in the bus is not already occupied
    for(addr_t i=start;i<=end;++i){
		M_REQUIRE(bus[i]==NULL, ERR_ADDRESS, "The adress range (%lu - %lu) is already occupied", start, end);
    }
    //Connects the component to the bus with offset 0 and return corresponding error
    return bus_forced_plug(bus,c,start,end,0);

}

int bus_unplug(bus_t bus, component_t* c){
    
    M_REQUIRE_NON_NULL(c);
    M_REQUIRE_NON_NULL(bus);
    
    //If component is plugged
    if(!(c->start==0 && c->end==0)){
        for(addr_t i=c->start;i<=c->end;++i){
            bus[i]=NULL;
        }
        c->start=0;
        c->end=0;
    }
    
    return ERR_NONE;
}
 
int bus_read(const bus_t bus, addr_t address, data_t* data){
    
    M_REQUIRE_NON_NULL(data);
    M_REQUIRE_NON_NULL(bus);

	*data = (bus[address] == NULL) ? DEFAULT_DATA : *bus[address];
    
    return ERR_NONE;
}

int bus_read16(const bus_t bus, addr_t address, addr_t* data16){
   
    M_REQUIRE_NON_NULL(data16);
    M_REQUIRE_NON_NULL(bus);
    
	M_REQUIRE(address<BUS_SIZE-1, ERR_ADDRESS, "The adress (%lu) is bigger than the bus_size-1 (%lu)", address, BUS_SIZE-1);

    *data16 = (bus[address] == NULL) ? DEFAULT_DATA : merge8(*bus[address], *bus[address+1]); //Little endian

    return ERR_NONE;
}

int bus_write(bus_t bus, addr_t address, data_t data){
    
    
    M_REQUIRE_NON_NULL(bus);
    M_REQUIRE_NON_NULL(bus[address]);
    
    *bus[address]=data;
    
    return ERR_NONE;
}

int bus_write16(bus_t bus, addr_t address, addr_t data16){
	
    
    
    M_REQUIRE_NON_NULL(bus);
    M_REQUIRE_NON_NULL(bus[address]);
    
    M_REQUIRE(address<BUS_SIZE-1, ERR_ADDRESS, "The adress (%lu) is bigger than the bus_size-1 (%lu)", address, BUS_SIZE-1);

    
    //little endian
    *bus[address]=lsb8(data16);
    *bus[address+1]=msb8(data16);
    
    return ERR_NONE;
}
