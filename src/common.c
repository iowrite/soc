#include <stdio.h>



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
void matrix_multiply(int *A, int *B, int *C, int m, int n, int p) 
{
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            C[i * p + j] = 0;
            for (int k = 0; k < n; k++) {
                C[i * p + j] += A[i * n + k] * B[k * p + j];
            }
        }
    }
}


/**
 * @brief inverse matrix
 *
 */
int inverse_matrix_2x2(float A[2][2], float inv_A[2][2]) 
{
    float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];

    if (det == 0) {
        return -1; 
    }

    inv_A[0][0] = A[1][1] / det;
    inv_A[0][1] = -A[0][1] / det;
    inv_A[1][0] = -A[1][0] / det;
    inv_A[1][1] = A[0][0] / det;

    return 0; 
}







// void print_matrix(int *matrix, int rows, int cols) {
//     for (int i = 0; i < rows; i++) {
//         for (int j = 0; j < cols; j++) {
//             printf("%d ", matrix[i * cols + j]);
//         }
//         printf("\n");
//     }
// }

// int main() {
//     // 定义矩阵
//     int A[1][2] = {{1, 2}};  // 1行2列
//     int B[2][1] = {{3}, {4}};  // 2行1列
//     int C[2][2] = {{5, 6}, {7, 8}};  // 2行2列

//     // 结果矩阵
//     int result1[1][1];  // A * B 的结果
//     int result2[1][2];  // A * C 的结果
//     int result3[2][1];  // B * C 的结果

//     // 计算 A * B
//     matrix_multiply((int *)A, (int *)B, (int *)result1, 1, 2, 1);
//     printf("A * B:\n");
//     print_matrix((int *)result1, 1, 1);

//     // 计算 A * C
//     matrix_multiply((int *)A, (int *)C, (int *)result2, 1, 2, 2);
//     printf("A * C:\n");
//     print_matrix((int *)result2, 1, 2);

//     // 计算 B * C
//     matrix_multiply((int *)B, (int *)C, (int *)result3, 2, 1, 2);
//     printf("B * C:\n");
//     print_matrix((int *)result3, 2, 1);

//     return 0;
// }