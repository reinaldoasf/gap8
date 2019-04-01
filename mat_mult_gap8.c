/* File:     
 *     pth_mat_vect.c 
 *
 * Purpose:  
 *     Computes a parallel matrix-vector product.  Matrix
 *     is distributed by block rows.  Vectors are distributed by 
 *     blocks.
 *
 * Input:
 *     m, n: order of matrix
 *     A, x: the matrix and the vector to be multiplied
 *
 * Output:
 *     y: the product vector
 *
 * Compile:  gcc -g -Wall -o pth_mat_vect pth_mat_vect.c -lpthread
 * Usage:
 *     pth_mat_vect <thread_count>
 *
 * Notes:  
 *     1.  Local storage for A, x, y is dynamically allocated.
 *     2.  Number of threads (thread_count) should evenly divide both 
 *         m and n.  The program doesn't check for this.
 *     3.  We use a 1-dimensional array for A and compute subscripts
 *         using the formula A[i][j] = A[i*n + j]
 *     4.  Distribution of A, x, and y is logical:  all three are 
 *         globally shared.
 *
 * IPP:    Section 4.3 (pp. 159 and ff.).  Also Section 4.10 (pp. 191 and 
 *         ff.)
 */

#include "cmsis.h"
#include "gap_common.h"
#include "mbed_wait_api.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>
#include <time.h>

#define FC_FREQ       (300000000)
#define CLUSTER_FREQ  (200000000)
#define F_DIV         (1000000)
#define NPOINTS       (10000000)
//#define NUM_THREADS   (8)
#define CORE_NUMBER   (8)
#define DATA_MAX      (10)

/* Global variables */
int     thread_count = CORE_NUMBER;
int     m, n;
int*    A;
int*    x;
int*    y;
/*------------------------------------------------------------------
 * Function:       pth_mat_vect
 * Purpose:        Multiply an mxn matrix by an nx1 column vector
 * In arg:         rank
 * Global in vars: A, x, m, n, thread_count
 * Global out var: y
 */
void pth_mat_vect() {
   int my_rank = __core_ID();
   int i, j;
   int local_m = m/thread_count; 
   int my_first_row = my_rank*local_m;
   int my_last_row = (my_rank+1)*local_m - 1;

   for (i = my_first_row; i <= my_last_row; i++) {
      y[i] = 0.0;
      for (j = 0; j < n; j++)
          y[i] += A[i*n+j]*x[j];
   }
   printf("Core %d - FirstRow: %d - LastRow: %d\n", my_rank, my_first_row, my_last_row);
}  /* pth_mat_vect */


/*------------------------------------------------------------------
 * Function:    Print_matrix
 * Purpose:     Print the matrix
 * In args:     title, A, m, n
 */
void print_matrix( char* title, int A[], int m, int n) {
   int   i, j;

   printf("%s\n", title);
   for (i = 0; i < m; i++) {
      for (j = 0; j < n; j++)
         printf("%d ", A[i*n + j]);
      printf("\n");
   }
}  /* Print_matrix */


/*------------------------------------------------------------------
 * Function:    Print_vector
 * Purpose:     Print a vector
 * In args:     title, y, m
 */
void print_vector(char* title, int y[], int m) {
   int   i;

   printf("%s\n", title);
   for (i = 0; i < m; i++)
      printf("%d ", y[i]);
   printf("\n");
}  /* Print_vector */

uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}

void Master_Entry()
{
    CLUSTER_CoresFork(pth_mat_vect, NULL);
}


void generate_data(){
    for (int j = 0; j < n; j++) {
        x[j] = rand() % DATA_MAX;
        for (int i = 0; i < m; i++) {
           A[i*n+j]  = rand() % DATA_MAX;
        }
    }
}

/*------------------------------------------------------------------*/
int main() {

    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 0);
    m = 24;
    n = 10;

    A = malloc(m*n*sizeof(int));
    x = malloc(n*sizeof(int));
    y = malloc(m*sizeof(int));

    srand(10);
    generate_data();

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);


    if (FLL_SetFrequency(uFLL_CLUSTER, CLUSTER_FREQ, 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    }

    printf("FC Frequency: %d MHz - Cluster Frequency: %d MHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV, FLL_GetFrequency(uFLL_CLUSTER)/F_DIV, current_voltage());

    CLUSTER_SendTask(0, Master_Entry, NULL, 0);
    printf("Waiting...\n");

    CLUSTER_Wait(0);

   print_matrix("The matrix is", A, m, n);
   print_vector("The vector is", x, n);
   print_vector("The product is", y, m);

   free(A);
   free(x);
   free(y);

   exit(0);
}  /* main */

