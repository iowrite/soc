#ifndef     _SOX_COMMON_H
#define     _SOX_COMMON_H

// C = A x B
/**
 * @brief matrix multiply
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
 * @brief inverse matrix
 *
 */
int inverse_matrix_2x2(float A[2][2], float inv_A[2][2]); 




#endif