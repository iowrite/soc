#include <stdint.h>
#include <stdio.h>
#ifndef     _SOX_COMMON_H
#define     _SOX_COMMON_H

#define UNUSED(x) (void)(x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))




/**
 * @brief matrix multiply, C = A x B
 * @note A, B, C must be flatened
 * A: m x n
 * B: n x p
 * C: m x p
 * @param[in] A     point to flatened matrix A  
 * @param[in] B     point to flatened matrix B
 * @param[out] C    point to flatened matrix C
 * @param m     row of matrix A, column of matrix B
 * @param n     column of matrix A, row of matrix B
 * @param p     column of matrix B
 */
void matrix_multiply(float *A, float *B, float *C, int m, int n, int p);


/**
 * @brief inverse matrix, 2x2 matrix only
 * @param A 2x2 matrix to be inverted
 * @param inv_A 2x2 matrix to store the inverted result
 *
 */
int inverse_matrix_2x2(float A[2][2], float inv_A[2][2]); 


void bubbleSort_ascend_uint16(uint16_t *inputArr, uint16_t *outputArr, size_t size);

void bubbleSort_ascend_int16(int16_t *inputArr, int16_t *outputArr, size_t size);

void bubbleSort_ascend_float(float *inputArr, float *outputArr, size_t size);


void bubbleSort_ascend_duble(float *inputArr, float *outputArr, size_t size);



#endif

