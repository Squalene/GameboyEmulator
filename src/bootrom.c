#include <stdlib.h>
#include "bootrom.h"
#include "error.h"
#include "memory.h"
#include "bus.h"
#include "cartridge.h"
#include "component.h"
#include "gameboy.h"

int bootrom_init(component_t* c){
	M_REQUIRE_NON_NULL(c);
	
	//Create component bootrom
	M_EXIT_IF_ERR(component_create(c, MEM_SIZE(BOOT_ROM)));
	
	//Set its content to GAMEBOY_BOOT_ROM_CONTENT
	data_t bootRomContentTab[] = GAMEBOY_BOOT_ROM_CONTENT;
	
	for(size_t i=0; i<GAMEBOY_BOOT_ROM_SIZE; ++i){
		c->mem->memory[i] = bootRomContentTab[i];
	}
	
	return ERR_NONE;
}

int bootrom_bus_listener(gameboy_t* gameboy, addr_t addr){
	M_REQUIRE_NON_NULL(gameboy);
		
	if(addr == REG_BOOT_ROM_DISABLE && gameboy->boot == 1){
		//Unplug and free botRom
		int err = bus_unplug(gameboy->bus, &(gameboy->bootrom));
		component_free(&gameboy->bootrom);
		M_EXIT_IF_ERR(err); //Return only now, so that we can free the component before
		//Plug the cartridge in the bus
		M_EXIT_IF_ERR(cartridge_plug(&(gameboy->cartridge), gameboy->bus));
		gameboy->boot = 0;
	}

	return ERR_NONE;
}
