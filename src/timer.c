#include <stdio.h>
#include "timer.h"
#include "error.h"
#include "cpu-storage.h"
#include "gameboy.h"
#include "bit.h"
#include "cpu.h"
#include "memory.h"


#define TIMER_ACTIVE_BIT 2

uint8_t getTIMAIncrementIndex(data_t TAC);
bit_t timer_state(gbtimer_t* timer);
int timer_incr_if_state_change(bit_t previousTimerState, gbtimer_t* currentTimer);

int timer_init(gbtimer_t* timer, cpu_t* cpu){
    
    M_REQUIRE_NON_NULL(timer);
    M_REQUIRE_NON_NULL(cpu);
    
    timer->cpu=cpu;
    timer->counter=0;
    
    return ERR_NONE;
}


int timer_cycle(gbtimer_t* timer){
    
    M_REQUIRE_NON_NULL(timer);
    
    bit_t previousTimerState =timer_state(timer);
    timer->counter+=GB_TICS_PER_CYCLE;
    //Updates the value in the bus
    M_EXIT_IF_ERR(cpu_write_at_idx(timer->cpu, REG_DIV, msb8(timer->counter)));
    
    //if the timer state has changed from true to false increment secondary timer
    M_EXIT_IF_ERR(timer_incr_if_state_change(previousTimerState, timer));
    
    return ERR_NONE;
}

int timer_bus_listener(gbtimer_t* timer, addr_t addr){
    
    M_REQUIRE_NON_NULL(timer);
    bit_t previousTimerState=timer_state(timer);
    
    switch (addr) {
        case REG_DIV:{
            timer->counter=0;
            M_EXIT_IF_ERR(cpu_write_at_idx(timer->cpu, REG_DIV, msb8(timer->counter)));
            
        }break;
            
        case REG_TAC:{
            //Do nothing
        }break;
            
        default:
            break;
    }

    M_EXIT_IF_ERR(timer_incr_if_state_change(previousTimerState, timer));
    
    return ERR_NONE;
}

/**
* @brief Returns 1 if the timer state is active and 0 otherwise
*
* @param timer the timer
* @return 1 if the timer state is active and 0 otherwise
*/
bit_t timer_state(gbtimer_t* timer){
    
    data_t TAC=cpu_read_at_idx(timer->cpu, REG_TAC);
    
    bit_t timerActive = bit_get(TAC, TIMER_ACTIVE_BIT);
    
    bit_t TIMAActive = bit_get16(timer->counter,getTIMAIncrementIndex(TAC));
    
    return timerActive & TIMAActive;
}

/**
* @brief Returns the index of bit of main counter we look at to increment secondary counter depending on current timer configuration
*
* @param TAC the value of timer configuration
* @return index of bit of main counter
*/
uint8_t getTIMAIncrementIndex(data_t TAC){
    //Extract two lsb
    switch (TAC & 3) {//0b11
        case 0:{//0b00
            return 9;
        }
            
        case 1:{//0b01
         return  3;
        }
            
        case 2:{//0b10
           return 5;
        }
            
        case 3:{//0b11
          return 7;
        }
            
        default:{
            return 0;
        }
    }
}

/**
* @brief Increments secondary timer if previousTimer state is true and actualTimerState is false
*
* @param previousTimerState: state of the previous timer
* @param currentTimerState: state of  the actualTimer
* @param cpu: the cpu
*/
int timer_incr_if_state_change(bit_t previousTimerState, gbtimer_t* currentTimer){
    
    M_REQUIRE_NON_NULL(currentTimer->cpu);
    if(previousTimerState && !timer_state(currentTimer)){
        data_t secondaryCounter = cpu_read_at_idx(currentTimer->cpu, REG_TIMA);
        secondaryCounter+=1;
        
        if(secondaryCounter == 0){//The counter has overflowed => launch Exception
            data_t reloadValueTMA = cpu_read_at_idx(currentTimer->cpu, REG_TMA);
            cpu_request_interrupt(currentTimer->cpu, TIMER);
            M_EXIT_IF_ERR(cpu_write_at_idx(currentTimer->cpu, REG_TIMA, reloadValueTMA));
        }
        else {
        M_EXIT_IF_ERR(cpu_write_at_idx(currentTimer->cpu, REG_TIMA, secondaryCounter));
        }
    }
    return ERR_NONE;
}




