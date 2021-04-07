#pragma once

/**
 * @file timer.h
 * @brief Game Boy Timer simulation header
 *
 * @author C. Hölzl, EPFL
 * @date 2019
 */

#include <stdint.h>//uint16_t
#include "cpu.h"//cpu_t
#include "memory.h"//addr_t

#ifdef __cplusplus
extern "C" {
#endif

// TIMER BUS REG ADDR

#define REG_DIV         0xFF04 //Address for 8MSB of primary timer
#define REG_TIMA        0xFF05 //Address for secondary timer (8bit)
#define REG_TMA         0xFF06 //Address for reinitialisation value of secondary timer
#define REG_TAC         0xFF07 //Address for timer configuration (8bit of which only the 3LSB are important)

#define TIMER_START     REG_DIV
#define TIMER_END       REG_TAC
#define TIMER_SIZE      ((REG_TAC-REG_DIV)+1)

/**
 * @brief Timer type
 */

typedef struct {
    cpu_t *cpu;
    uint16_t counter;
    
} gbtimer_t;

/**
 * @brief Initiates a timer
 *
 * @param timer timer to initiate
 * @param cpu cpu to use for timer
 * @return error code
 */
int timer_init(gbtimer_t* timer, cpu_t* cpu);


/**
 * @brief Run one Timer cycle
 *
 * @param timer timer to cycle
 * @return error code
 */
int timer_cycle(gbtimer_t* timer);


/**
 * @brief Timer bus listening handler
 *
 * @param timer timer
 * @param address trigger address
 * @return error code
 */
int timer_bus_listener(gbtimer_t* timer, addr_t addr);

#ifdef __cplusplus
}
#endif
