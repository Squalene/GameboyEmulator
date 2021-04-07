#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>//for uint32_t and int64_t
#include <string.h>//for memset
#include "bit_vector.h"
#include "util.h"//for zero_init_ptr
#include "image.h"//for IMAGE_LINE_WORD_BITS
#include "ourError.h"
#include "bit.h"

/**
 * @brief Represents a binary operation between two uint32_t
 */
typedef uint32_t (*binop)(uint32_t a, uint32_t b);

bit_vector_t* bit_vector_binaryOp(bit_vector_t* pbv1, const bit_vector_t* pbv2, binop op);
uint32_t xor32 (uint32_t a, uint32_t b);
uint32_t or32 (uint32_t a, uint32_t b);
uint32_t and32 (uint32_t a, uint32_t b);
bit_vector_t* bit_vector_resize(const bit_vector_t* pbv, size_t size);
uint32_t bit_vector_extract_subgroup32(const bit_vector_t* pbv, int64_t index);
uint64_t positive_modulo(int64_t nbr, uint64_t divider);
bit_vector_t* bit_vector_extract(const bit_vector_t* pbv, int64_t index, size_t size);
void bit_vector_extract_multiple32(const bit_vector_t* pbv, int64_t index, size_t size, bit_vector_t* result);
void bit_vector_extract_not_multiple32(const bit_vector_t* pbv, int64_t index, size_t size, bit_vector_t* result);
int printBinary(uint32_t number);

/**
 * @brief Compute logical XOR of two uint32_t
 * @param a first operand
 * @param b second operand
 * @return the logical XOR of the two operands
 */
uint32_t xor32 (uint32_t a, uint32_t b){
    return a^b;
}

/**
 * @brief Compute logical OR of two uint32_t
 * @param a first operand
 * @param b second operand
 * @return the logical OR of the two operands
 */
uint32_t or32 (uint32_t a, uint32_t b){
    return a|b;
}

/**
 * @brief Compute logical XOR of two uint32_t
 * @param a first operand
 * @param b second operand
 * @return the logical AND of the two operands
 */
uint32_t and32 (uint32_t a, uint32_t b){
    return a&b;
}

/**
 * @brief Do a division with the divider and compute the ceiling of the result
 */
#define INT_DIVISION_CEILING(size, divider)\
	((size%divider == 0) ? (size/divider): ((size/divider)+1))
	
/**
 * @brief Do a division with 32 (IMAGE_LINE_WORD_BITS) and compute the ceiling of the result
 */
#define NUMBER_32BIT_VECTORS(size)\
	INT_DIVISION_CEILING(size, IMAGE_LINE_WORD_BITS)

/**
 * @brief Return the number of elements in a bit_vector (same as NUMBER_32BIT_VECTORS but with vectors instead of numbers)
 */
#define NUMBER_OF_ELEMENTS(pbv)\
	NUMBER_32BIT_VECTORS(pbv->size)

/**
 * @brief Return the min of two values
 */
#define min(x,y)\
	((x)<=(y) ? (x) : (y))
/**
 * @brief Mask the last bits o the vector since we always use a multiple of 32, but some bits are unused
 * 
 * @param pbv pointer to the vector
 * 
 * @return the pointer to the vector
*/
bit_vector_t* maskLastUnusedBits(bit_vector_t* pbv){
	
	M_REQUIRE_NOT_NULL_RETURN_NULL(pbv);
	
	if(pbv->size%IMAGE_LINE_WORD_BITS!=0){
		size_t numberOfElements = NUMBER_OF_ELEMENTS(pbv);
		uint32_t maskLSB = (1 << (pbv->size%IMAGE_LINE_WORD_BITS))-1;//Create a mask of 0...111 to let the bits that we added to 0
		pbv->content[numberOfElements-1] = pbv->content[numberOfElements-1] & maskLSB;
	}

	return pbv;
}
	
bit_vector_t* bit_vector_create(size_t size, bit_t value){
	
	if(size == 0){
		fprintf(stderr, "The size should not be 0\n");
		return NULL;
	}
    
    size_t numberOfElements = NUMBER_32BIT_VECTORS(size);
    
    bit_vector_t* result = NULL;
    
    //Allocates memory for the structure
    if( numberOfElements > (((SIZE_MAX-sizeof(bit_vector_t))/sizeof(uint32_t)) +1) || (result= malloc(sizeof(bit_vector_t) + (numberOfElements-1) * sizeof(uint32_t)))==NULL){
        fprintf(stderr, "The memory allocation for the vector failed\n");
        return NULL;
    }
    
    //Initialize all memory to 0
    memset(result, 0, sizeof(bit_vector_t) + (numberOfElements-1) * sizeof(uint32_t)); //We use memset instead of zero_init_ptr, because it would not initialize data outside th struct (it would only intilaize content[0])
    result->size=size;
    
    if(value == 1){
        bit_vector_not(result);//Negating 0 sets all the bits to 1
		M_REQUIRE_NOT_NULL_RETURN_NULL(maskLastUnusedBits(result));
    }
    return result;
}


bit_vector_t* bit_vector_cpy(const bit_vector_t* pbv){
    
    M_REQUIRE_NOT_NULL_RETURN_NULL(pbv);
    
    bit_vector_t* newVect = bit_vector_create(pbv->size, 0);
    
    M_REQUIRE_NOT_NULL_RETURN_NULL(newVect);
    
    //Copy the values
    
    size_t numberOfElements = NUMBER_OF_ELEMENTS(pbv);
    for(size_t i=0;i<numberOfElements;++i){
        newVect->content[i]=pbv->content[i];
    }
    
    return newVect;
}


bit_t bit_vector_get(const bit_vector_t* pbv, size_t index){
    
    M_REQUIRE_NOT_NULL_RETURN(pbv,0);
    
    if(index>= pbv->size){
        fprintf(stderr, "The given index should not exceed vector size\n");
        return 0;
    }
    
    return bit_get32(pbv->content[index/IMAGE_LINE_WORD_BITS],index%IMAGE_LINE_WORD_BITS);
}

bit_vector_t* bit_vector_not(bit_vector_t* pbv){
    
    M_REQUIRE_NOT_NULL_RETURN_NULL(pbv);
    
    size_t numberOfElements = NUMBER_OF_ELEMENTS(pbv);

    for(size_t i=0;i<numberOfElements;++i){
        pbv->content[i]=~pbv->content[i];
    }
    
    maskLastUnusedBits(pbv);
    
    return pbv;
}

/**
 * @brief Compute binary operation on two bit_vectors and stores the result in the first bit_vector
 * @param pbv1 first operand
 * @param pbv2 second operand
 * @param op a binary operation between two uint32_t
 * @return the binary operation between the two bit_vectors
*/
bit_vector_t* bit_vector_binaryOp(bit_vector_t* pbv1, const bit_vector_t* pbv2, binop op){
    
    M_REQUIRE_NOT_NULL_RETURN_NULL(pbv1);
    M_REQUIRE_NOT_NULL_RETURN_NULL(pbv2);

    if(pbv1->size!=pbv2->size){
		fprintf(stderr, "pbv1-> size should be equal to pbv2->size (%lu != %lu)\n", pbv1->size, pbv2->size);
		return NULL;
    }
	
    size_t numberOfElements = NUMBER_OF_ELEMENTS(pbv1);
    for(size_t i=0;i<numberOfElements;++i){
      pbv1->content[i]= op(pbv1->content[i],pbv2->content[i]);
    }
    
    maskLastUnusedBits(pbv1); //Could be useful if we implemented a xnor for example

    return pbv1;
}

bit_vector_t* bit_vector_and(bit_vector_t* pbv1, const bit_vector_t* pbv2){
    
     return bit_vector_binaryOp(pbv1,pbv2,and32);
}

bit_vector_t* bit_vector_or(bit_vector_t* pbv1, const bit_vector_t* pbv2){
    
     return bit_vector_binaryOp(pbv1,pbv2,or32);
}

bit_vector_t* bit_vector_xor(bit_vector_t* pbv1, const bit_vector_t* pbv2){
    
    return bit_vector_binaryOp(pbv1,pbv2,xor32);
}

bit_vector_t* bit_vector_extract_zero_ext(const bit_vector_t* pbv, int64_t index, size_t size){
	
	if(pbv == NULL){
		return bit_vector_create(size, 0);
	}
	
	if(size == 0){
		fprintf(stderr, "The size should not be 0\n");
		return NULL;
	}
	
	bit_vector_t* result = bit_vector_create(size, 0);
	M_REQUIRE_NOT_NULL_RETURN_NULL(result);
	
	if(index%IMAGE_LINE_WORD_BITS == 0){
		 bit_vector_extract_multiple32(pbv, index, result->size, result);
	}
	else{
		 bit_vector_extract_not_multiple32(pbv, index, result->size, result);
	}
	
	return result;

}

bit_vector_t* bit_vector_extract_wrap_ext(const bit_vector_t* pbv, int64_t index, size_t size){


    M_REQUIRE_NOT_NULL_RETURN_NULL(pbv);
    
    int64_t numberBitsShiftRight=positive_modulo(index, pbv->size);
    
    bit_vector_t* firstExtendedCopy = bit_vector_resize(pbv,pbv->size);//extended copy on size
    M_REQUIRE_NOT_NULL_RETURN_NULL(firstExtendedCopy);
    
    bit_vector_t* firstRightShifted = bit_vector_shift(firstExtendedCopy,-numberBitsShiftRight);//Minus is for right shift
    bit_vector_free(&firstExtendedCopy);
    M_REQUIRE_NOT_NULL_RETURN_NULL(firstRightShifted);

    bit_vector_t* result = bit_vector_resize(firstRightShifted, size);//Resize it to the right size
    bit_vector_free(&firstRightShifted);
    M_REQUIRE_NOT_NULL_RETURN_NULL(result);
    
    size_t alreadyPlacedBits = min(pbv->size-numberBitsShiftRight, size);
    size_t copyNumber = INT_DIVISION_CEILING(size-alreadyPlacedBits,pbv->size);//Number of copies we have to do
    //Do the wrapping
    for(size_t i=0;i<copyNumber;++i){
        bit_vector_t* extendedCopy= bit_vector_resize(pbv,size);//extended copy on size
        M_REQUIRE_NOT_NULL_RETURN_NULL(extendedCopy);

        bit_vector_t* shiftedCopy= bit_vector_shift(extendedCopy,i*pbv->size+alreadyPlacedBits);//shift to the left
		bit_vector_free(&extendedCopy);
        M_REQUIRE_NOT_NULL_RETURN_NULL(shiftedCopy);

        bit_vector_t* tmp = bit_vector_or(result, shiftedCopy); //We use tmp to be able to free shiftedCopy if an error occured in or operation
        bit_vector_free(&shiftedCopy);
        M_REQUIRE_NOT_NULL_RETURN_NULL(tmp);
    }
    
    maskLastUnusedBits(result);
    return result;

}

bit_vector_t* bit_vector_shift(const bit_vector_t* pbv, int64_t shift){
	
	M_REQUIRE_NOT_NULL_RETURN_NULL(pbv);
	
	return bit_vector_extract_zero_ext(pbv, -shift, pbv->size);
}

bit_vector_t* bit_vector_join(const bit_vector_t* pbv1, const bit_vector_t* pbv2, int64_t shift){
    
    M_REQUIRE_NOT_NULL_RETURN_NULL(pbv1);
    M_REQUIRE_NOT_NULL_RETURN_NULL(pbv2);
    
    if(pbv1->size!=pbv2->size){
		fprintf(stderr, "pbv1-> size should be equal to pbv2->size (%lu != %lu)\n", pbv1->size, pbv2->size);
		return NULL;
    }
    
    if(shift<0 || shift>pbv1->size){
		fprintf(stderr, "Shift (=%ld) should be between 0 and size(%lu)\n", shift, pbv1->size);
        return NULL;
    }
    
    bit_vector_t* result= bit_vector_create(pbv1->size, 0);
    
    M_REQUIRE_NOT_NULL_RETURN_NULL(result);

    size_t numberOfElementsFirstVector=shift/IMAGE_LINE_WORD_BITS;
    
    for(size_t i=0;i<numberOfElementsFirstVector;++i){
        result->content[i]= pbv1->content[i];
    }
    
    result->content[numberOfElementsFirstVector]= bit_join32(pbv1->content[numberOfElementsFirstVector], pbv2->content[numberOfElementsFirstVector], shift%IMAGE_LINE_WORD_BITS);
    
    size_t numberOfElementsPbv1 = NUMBER_OF_ELEMENTS(pbv1);
    for(size_t i=numberOfElementsFirstVector+1;i<numberOfElementsPbv1;++i){
        result->content[i]= pbv2->content[i];
    }
    
    return result;
}

int bit_vector_print(const bit_vector_t* pbv){
    
    M_REQUIRE_NOT_NULL_RETURN(pbv,0);
    
    int totalChar=0;
    
    size_t numberOfElements = NUMBER_OF_ELEMENTS(pbv);

    for(size_t i=numberOfElements; (i--)>0;){
        totalChar+=printBinary(pbv->content[i]);
    }
    return totalChar;
}

int bit_vector_println(const char* prefix, const bit_vector_t* pbv){
    
    M_REQUIRE_NOT_NULL_RETURN(pbv,0);
    M_REQUIRE_NOT_NULL_RETURN(prefix,0);

   
    int totalChar=0;
    
    totalChar+=printf("%s",prefix);
    totalChar+=bit_vector_print(pbv);
    totalChar+=printf("\n");
    
    return totalChar;
}

void bit_vector_free(bit_vector_t** pbv){
    
    if(pbv!=NULL){
        free(*pbv);
        *pbv=NULL;
    }
}


/**
 * @brief Return the group of 32 bits containing the index
 *
 * @param pbv: pointer to bit vector
 * @param index: index
 *
 * @return the group of 32 bits containing the index
 */
uint32_t bit_vector_extract_subgroup32(const bit_vector_t* pbv, int64_t index){
	
	M_REQUIRE_NOT_NULL_RETURN(pbv, 0);

	if(0<=index && index<NUMBER_OF_ELEMENTS(pbv)*IMAGE_LINE_WORD_BITS){
		return pbv->content[index/IMAGE_LINE_WORD_BITS];
	}
	else {
		return 0;
	}
}

/**
 * @brief Return a modulo with positive result (useful when the number is negativ)
 *
 * @param nbr: numerator
 * @param divider: divider (must be positive, unsigned)
 *
 * @return the positive modulo
 */
uint64_t positive_modulo(int64_t nbr, uint64_t divider){
	int64_t modulo_possibly_neg = nbr%divider;
	return modulo_possibly_neg>=0 ? modulo_possibly_neg : modulo_possibly_neg+divider;
}

/**
 * @brief compute the extraction by modifing vector result when the index is a multiple of 32, or do nothing if it is not
 *
 * @param pbv: pointer to bit vector to extract from
 * @param index: index (must be a multiple of 32, otherwise do nothing)
 * @param size: size of the new vector that we will extract
 * @param result: pointer to reslut bit vector that we want to modify
 */
void bit_vector_extract_multiple32(const bit_vector_t* pbv, int64_t index, size_t size, bit_vector_t* result){
	if(pbv != NULL && result!=NULL && index%IMAGE_LINE_WORD_BITS == 0){
		size_t numberOfElements = NUMBER_32BIT_VECTORS(size);
		for(size_t i = 0; i<numberOfElements; ++i){
			result->content[i] = bit_vector_extract_subgroup32(pbv, index);
			index += IMAGE_LINE_WORD_BITS;
		}
	}
}

/**
 * @brief compute the extraction by modifing vector result when the index is not a multiple of 32, or do nothing if it is
 *
 * @param pbv: pointer to bit vector to extract from
 * @param index: index (must not be a multiple of 32, otherwise do nothing)
 * @param size: size of the new vector that we will extract
 * @param result: pointer to reslut bit vector that we want to modify
 */
void bit_vector_extract_not_multiple32(const bit_vector_t* pbv, int64_t index, size_t size, bit_vector_t* result){
	if(pbv!=NULL && result!=NULL && index%IMAGE_LINE_WORD_BITS != 0){
		
		int8_t shift = index%IMAGE_LINE_WORD_BITS;
		uint32_t firstPart = bit_vector_extract_subgroup32(pbv, index);

		size_t numberOfElements = NUMBER_32BIT_VECTORS(size);

		for(size_t i = 0; i<numberOfElements; ++i){
			
			uint32_t secondPart = bit_vector_extract_subgroup32(pbv, index+IMAGE_LINE_WORD_BITS);
			
			if(shift<0){
				result->content[i] = (firstPart>>(IMAGE_LINE_WORD_BITS+shift)) | (secondPart<<(-shift));
			}
			else{
				result->content[i] = (firstPart>>shift) | (secondPart<<(IMAGE_LINE_WORD_BITS-shift));
			}
			
			firstPart = secondPart;
			index += IMAGE_LINE_WORD_BITS;

		}
	}
}

/**
 * @brief Resize a bit_vector
 * 
 * @param pbv: pointer to bit vector we want to extend
 * @param size: the size of the new vector.
 *
 * @return a new vector resized to the given size.
 */
bit_vector_t* bit_vector_resize(const bit_vector_t* pbv, size_t size){
    
    M_REQUIRE_NOT_NULL_RETURN_NULL(pbv);
    
    return bit_vector_extract_zero_ext(pbv, 0, size);
}


/**
 * @brief print a 32 bit unsigned number in binary format.
 *
 * @param number: the number to print.
 *
 * @return the total number of chars printed.
 */
int printBinary(uint32_t number){
	
	int totalChar = 0;
	
	for(size_t i=IMAGE_LINE_WORD_BITS; (i--)>0;){
		if(number&(1<<i)){
			totalChar += printf("1");
		}
		else{
			totalChar += printf("0");
		}
	}
	
	return totalChar;
}
