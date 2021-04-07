#pragma once

/**
 * @file ourError.h
 * @brief Error codes for Gameboy Emulator
 *
 * @author Antoine Masanet, Loïc Houmard
 * @date 2020
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "error.h"

#define M_PRINT_NOT_NULL(arg)\
fprintf(stderr,"%s should not be NULL \n",arg);

#define M_PRINT_ERROR(error_code)\
fprintf(stderr,"Error: %s \n", ERR_MESSAGES[error_code - ERR_NONE]);

#define M_PRINT_IF_ERROR(error_code, msg)\
do { \
    if(error_code!=ERR_NONE){ \
        fprintf(stderr,msg"\n");\
    }\
}while(0)

#define M_REQUIRE_NOT_NULL_RETURN(arg, ret)\
do { \
    if(arg==NULL){\
        M_PRINT_NOT_NULL(#arg);\
        return ret;\
    }\
}while(0)

#define M_REQUIRE_NOT_NULL_RETURN_NULL(arg)\
	M_REQUIRE_NOT_NULL_RETURN(arg, NULL)

#ifdef __cplusplus
}
#endif
