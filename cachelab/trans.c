/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

#define BSIZE 8
#define BSIZE_64 4

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void block_trans_32(int ii, int jj, int A[32][32], int B[32][32]);
void block_trans_64(int ii, int jj, int A[64][64], int B[64][64]);

void trans_32(int A[32][32], int B[32][32]);
void trans_64(int A[64][64], int B[64][64]);
void trans_otherwise(int M, int N, int A[N][M], int B[M][N]);


/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
  if (M == 32 && N == 32)
    trans_32(A, B);
  else if (M == 64 && N == 64)
    trans_64(A, B);
  else
    trans_otherwise(M, N, A, B);
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

void block_trans_simple(
  int M, int N,
  int ii, int jj, int size,
  int A[N][M], int B[M][N])
{
  int i, j;
  int ibound, jbound;

  ibound = ii + size;
  jbound = jj + size;
 
  for (i = ii; i < ibound; i++)
    for (j = jj; j < jbound; j++)
      B[j][i] = A[i][j];
}

void block_trans_32(int ii, int jj, int A[32][32], int B[32][32])
{
  int i, j;

  /* Store diagonal element because they evict each other. */
  for (i = ii; i < ii + BSIZE; i++) {
    for (j = jj; j < jj + BSIZE; j++)
      if ((i - ii) != (j - jj))
        B[j][i] = A[i][j];

    B[jj + i - ii][i] = A[i][jj + i - ii];
  }
}

void block_trans_64(int ii, int jj, int A[64][64], int B[64][64])
{
  int i, j;
  int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;

  /* Store diagonal element because they evict each other.
   * And use tmp variables to store two lines */
  for (i = ii; i < ii + BSIZE_64; i++) {
    for (j = jj; j < jj + BSIZE_64; j++)
      if ((i - ii) != (j - jj))
        B[j][i] = A[i][j];

    if (i == ii) {
      tmp0 = A[i][j++];
      tmp1 = A[i][j++];
      tmp2 = A[i][j++];
      tmp3 = A[i][j++];
    }

    if (i - ii == 1) {
      tmp4 = A[i][j++];
      tmp5 = A[i][j++];
      tmp6 = A[i][j++];
      tmp7 = A[i][j++];
    }

    B[jj + i - ii][i] = A[i][jj + i - ii];
  }

  /* Proceed on U shape */
  ii += BSIZE_64;
  for (i = ii; i < ii + BSIZE_64; i++) {
    for (j = jj; j < jj + BSIZE_64; j++)
      if ((i - ii) != (j - jj))
        B[j][i] = A[i][j];

    B[jj + i - ii][i] = A[i][jj + i - ii];
  }
 
  jj += BSIZE_64;
  for (i = ii; i < ii + BSIZE_64; i++) {
    for (j = jj; j < jj + BSIZE_64; j++)
      if ((i - ii) != (j - jj))
        B[j][i] = A[i][j];

    B[jj + i - ii][i] = A[i][jj + i - ii];
  }

  ii -= BSIZE_64;
  for (i = ii + 2; i < ii + BSIZE_64; i++) {
    for (j = jj; j < jj + BSIZE_64; j++)
      if ((i - ii) != (j - jj))
        B[j][i] = A[i][j];

    B[jj + i - ii][i] = A[i][jj + i - ii];
  }

  /* Stored two lines */
  i = ii;
  j = jj;

  B[j++][i] = tmp0;
  B[j++][i] = tmp1;
  B[j++][i] = tmp2;
  B[j++][i] = tmp3;

  i = ii + 1;
  j = jj;

  B[j++][i] = tmp4;
  B[j++][i] = tmp5;
  B[j++][i] = tmp6;
  B[j++][i] = tmp7;
}

void trans_32(int A[32][32], int B[32][32])
{
  int ii, jj;

  for (ii = 0; ii < 32; ii += BSIZE) {
    for (jj = 0; jj < 32; jj += BSIZE) {
      block_trans_32(ii, jj, A, B);    
    }
  }
}

void trans_64(int A[64][64], int B[64][64])
{
  int ii, jj;
  
  for (ii = 0; ii < 64; ii += BSIZE) {
    for (jj = 0; jj < 64; jj += BSIZE) {
      block_trans_64(ii, jj, A, B);
    }
  }
}


char trans_otherwise_desc[] = "Simple blocked transpose";
void trans_otherwise(int M, int N, int A[N][M], int B[M][N])
{
  int jj, i, j;
  int block_bound;
  int EM = BSIZE * (M / BSIZE);

  for (jj = 0; jj < EM; jj += BSIZE) {
    for (i = 0; i < N; i++) {
      block_bound = jj + BSIZE;

      for (j = jj; j < block_bound; j++) {
        B[j][i] = A[i][j];
      }
    }
  }    

  for (i = 0; i < N; i++) {
    for (j = jj; j < M; j++) {
      B[j][i] = A[i][j];
    }
  }

}


/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

