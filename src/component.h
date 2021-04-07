#pragma once

/**
 * @file component.h
 * @brief Game Boy Component simulation header
 *
 * @author C. Hölzl, EPFL
 * @date 2019
 */


#include <stdint.h>//
#include <stdio.h>//
#include <stdlib.h>//size_t
#include "memory.h"//addr_t and memory_t

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Free the component if an error occured and return the error
 *
 */
#define COMPONENT_FREE_IF_ERROR(err,component)\
    if(err!=ERR_NONE){\
        component_free(component);\
        return err;\
    }
/**
 * @brief Component data structure.
 *        Holds a pointer to its memory and start/end addresses
 */

typedef struct {
    memory_t* mem;
    addr_t start;
    addr_t end;
} component_t;


/**
 * @brief Creates a component given various arguments
 *
 * @param c component pointer to initialize
 * @param mem_size size of the memory of the component
 * @return error code
 */
int component_create(component_t* c, size_t mem_size);

/**
 * @brief Shares memory between two components
 *
 * @param c component pointer to share to
 * @param c_old component to share from
 * @return error code
 */
int component_shared(component_t* c, component_t* c_old);

/**
 * @brief Destroy's a component
 *
 * @param c component pointer to destroy
 */
void component_free(component_t* c);

#ifdef __cplusplus
}
#endif
