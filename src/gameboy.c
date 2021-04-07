#include <stdio.h>
#include "component.h"
#include "bus.h"
#include "error.h"
#include "gameboy.h"
#include "util.h"
#include "bootrom.h"
#include "timer.h"
#include "cartridge.h"
#include "ourError.h"
#include "cpu.h"

#ifdef BLARGG
#include "cpu-storage.h" //cpu_read_at_idx
#include "memory.h" //data_t
#endif

/**
 * @brief Create a component and plug it
 *
 */
#define CREATE_AND_PLUG(X, gameboy)\
	do {\
		GAMEBOY_FREE_IF_ERROR(component_create(&(gameboy->components[X ## _INDEX]), MEM_SIZE(X)), gameboy);\
		GAMEBOY_FREE_IF_ERROR(bus_plug(gameboy->bus, &gameboy->components[X ## _INDEX], X ## _START, X ## _END), gameboy);\
        ++gameboy->nb_components;\
    } while(0)
 
/**
 * @brief Free the gameboy if an error occured and return the error
 *
 */
#define GAMEBOY_FREE_IF_ERROR(err,gameboy)\
    if(err!=ERR_NONE){\
        gameboy_free(gameboy);\
        return err;\
    }
   
/**
 * @brief Number of cycles needed to draw on screen
 *
 */ 
#define CYCLES_GAMEBOY_DRAW 17556

#ifdef BLARGG
static int blargg_bus_listener(gameboy_t* gameboy, addr_t addr);
#endif

int gameboy_create(gameboy_t* gameboy,const char* filename){
    
    M_REQUIRE_NON_NULL(gameboy);
    zero_init_ptr(gameboy);//Initialize all fields of gameboy to 0, arrays included
    //Set boot to 1 to initialize the gameboy
    gameboy->boot = 1;
   
    //WorkRam
    CREATE_AND_PLUG(WORK_RAM, gameboy);


	//EchoRam
    component_t echoRam;
    //Create echoRam without memory (will later point to the same as workRam)
    GAMEBOY_FREE_IF_ERROR(component_create(&echoRam, 0), gameboy);
    //EchoRam will now point to same memory as workRam
    GAMEBOY_FREE_IF_ERROR(component_shared(&echoRam, &gameboy->components[WORK_RAM_INDEX]), gameboy);
    //plug echoRam
    GAMEBOY_FREE_IF_ERROR(bus_plug(gameboy->bus, &echoRam, ECHO_RAM_START, ECHO_RAM_END),gameboy);
    
    
    //Create registers
    CREATE_AND_PLUG(REGISTERS, gameboy); 
    
    //Create extern_ram
    CREATE_AND_PLUG(EXTERN_RAM, gameboy); 
    
    //Create video_ram
    CREATE_AND_PLUG(VIDEO_RAM, gameboy); 
    
    //Create graph_ram
    CREATE_AND_PLUG(GRAPH_RAM, gameboy); 
    
     //Create useless
    CREATE_AND_PLUG(USELESS, gameboy);
    
    //Plug the cpu of the gameboy
    M_EXIT_IF_ERR(cpu_init(&gameboy->cpu));
    M_EXIT_IF_ERR(cpu_plug(&gameboy->cpu, &gameboy->bus));
    
	
    //Create the cartridge and plug it in the bus
	GAMEBOY_FREE_IF_ERROR(cartridge_init(&(gameboy->cartridge), filename), gameboy);
	GAMEBOY_FREE_IF_ERROR(cartridge_plug(&(gameboy->cartridge), gameboy->bus), gameboy);
	
    //Initialise bootRom and plug it=> override some of the first data_t of the cartridge
    GAMEBOY_FREE_IF_ERROR(bootrom_init(&(gameboy->bootrom)),gameboy);
    GAMEBOY_FREE_IF_ERROR(bootrom_plug(&(gameboy->bootrom), gameboy->bus), gameboy);
    
    //Initialize the timer
    GAMEBOY_FREE_IF_ERROR(timer_init(&gameboy->timer, &gameboy->cpu), gameboy);
    
    //Initialize the screen and plug it
    GAMEBOY_FREE_IF_ERROR(lcdc_init(gameboy),gameboy);
    GAMEBOY_FREE_IF_ERROR(lcdc_plug(&(gameboy->screen), gameboy->bus),gameboy);
    
    //Initialize the joypad
    GAMEBOY_FREE_IF_ERROR(joypad_init_and_plug(&(gameboy->pad),&(gameboy->cpu)),gameboy);
    
    return ERR_NONE;
}

void gameboy_free(gameboy_t* gameboy){
    if(gameboy!=NULL){
				
        for (size_t i=0;i<GB_NB_COMPONENTS;++i){
			M_PRINT_IF_ERROR(bus_unplug(gameboy->bus, &(gameboy->components[i])), "Could not unplug a component");
			component_free(&(gameboy->components[i]));
        }
        
        // Also set the pointers in the bus in the range of echoRam to NULL
        for (size_t i=ECHO_RAM_START;i<=ECHO_RAM_END;++i){
            gameboy->bus[i]=NULL;
        }
        
        M_PRINT_IF_ERROR(bus_unplug(gameboy->bus, &(gameboy->cartridge.c)), "Could not unplug cartridge");
        cartridge_free(&(gameboy->cartridge));
        //In case an error occures while bootrom is plugged in
        M_PRINT_IF_ERROR(bus_unplug(gameboy->bus, &(gameboy->bootrom)), "Could not unplug bootrom");
        component_free(&gameboy->bootrom);
        
        lcdc_free(&gameboy->screen);
        
        zero_init_ptr(gameboy);
    }
}

int gameboy_run_until(gameboy_t* gameboy, uint64_t cycle){
	
	M_REQUIRE_NON_NULL(gameboy);
	
	for(uint64_t i=gameboy->cycles; i<cycle; ++i){
		
		
		#ifdef BLARGG
		//Throw a VBlank interrupt every 17’556 cycles
		if((gameboy->cycles>0) && ((gameboy->cycles % CYCLES_GAMEBOY_DRAW) == 0)){
			cpu_request_interrupt(&(gameboy->cpu), VBLANK);
		}
		#endif
		
		M_EXIT_IF_ERR(lcdc_cycle(&(gameboy->screen), gameboy->cycles));

        M_EXIT_IF_ERR(timer_cycle(&gameboy->timer));
		M_EXIT_IF_ERR(cpu_cycle(&(gameboy->cpu)));
		++(gameboy->cycles);
		M_EXIT_IF_ERR(bootrom_bus_listener(gameboy, gameboy->cpu.write_listener));
        M_EXIT_IF_ERR(timer_bus_listener(&gameboy->timer, gameboy->cpu.write_listener));
        M_EXIT_IF_ERR(lcdc_bus_listener(&(gameboy->screen), gameboy->cpu.write_listener));
        M_EXIT_IF_ERR(joypad_bus_listener(&(gameboy->pad), gameboy->cpu.write_listener));
		
        #ifdef BLARGG
        M_EXIT_IF_ERR(blargg_bus_listener(gameboy, gameboy->cpu.write_listener));
        #endif
        
	}

	

	
	return ERR_NONE;
}

#ifdef BLARGG
static int blargg_bus_listener(gameboy_t* gameboy, addr_t addr){
	M_REQUIRE_NON_NULL(gameboy);
	
	if(addr == BLARGG_REG){
		data_t data = cpu_read_at_idx(&(gameboy->cpu), addr);
		printf("%c", data);
	}
	
	return ERR_NONE;
}
#endif
