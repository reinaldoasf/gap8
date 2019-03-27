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
#define NUM_THREADS   (8)
#define CORE_NUMBER   (8)

uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}


int     thread_count;
int     m, n;
double* A;
double* x;
double* y;

/* Serial functions */
void Usage(char* prog_name);
void Read_matrix(char* prompt, double A[], int m, int n);
void Read_vector(char* prompt, double x[], int n);
void Print_matrix(char* title, double A[], int m, int n);
void Print_vector(char* title, double y[], double m);

/* Parallel function */
void *mat_vect(void* rank);

void Master_Entry(int * L1_mem)
{
    CLUSTER_CoresFork(mat_vect, (void *) L1_mem);
}





int main()
{
	long       thread;
   int* thread_handles;

   if (argc != 2) Usage(argv[0]);
   thread_count = atoi(argv[1]);
   thread_handles = malloc(thread_count*sizeof(int);


	printf("Enter m and n\n");
	scanf("%d%d", &m, &n);
   A = malloc(m*n*sizeof(double));
   x = malloc(n*sizeof(double));
   y = malloc(m*sizeof(double));
   
   Read_matrix("Enter the matrix", A, m, n);
   Print_matrix("We read", A, m, n);

   Read_vector("Enter the vector", x, n);
   Print_vector("We read", x, n);


    int *L1_mem = L1_Malloc(8);
    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 0);

	CLUSTER_Start(0,CORE_NUMBER);
	 if (FLL_SetFrequency(uFLL_CLUSTER, CLUSTER_FREQ, 0) == -1)
	 {
		printf("Error of changing frequency, check Voltage value!\n");
    }
	printf("FC Frequency: %d MHz - Cluster Frequency: %d MHz - Voltage: %lu mV\n",
		    FLL_GetFrequency(uFLL_SOC)/F_DIV, FLL_GetFrequency(uFLL_CLUSTER)/F_DIV, current_voltage());

	CLUSTER_SendTask(0, Master_Entry, (void *) thread_hanles[thread], 0);
	    printf("Waiting...\n");

	CLUSTER_Wait(0);
   for (thread = 0; thread < thread_count; thread++)
      pthread_create(&thread_handles[thread], NULL,
         Pth_mat_vect, (void*) thread);

	CLUSTER_Wait(0);

   Print_vector("The product is", y, m);


	for (int i = 0; i < 8; i++) {
	count += L1_mem[i];
	printf("CORE %d - count = %d\n", i, L1_mem[i]);
	}

/* Cluster Stop - Power down */
	CLUSTER_Stop(0);

	printf("DONE!\n");
	
   Print_vector("The product is", y, m);
	free(A);
	free(x);
	free(y);


	exit(0);
	

}


void Usage (char* prog_name) {
   fprintf(stderr, "usage: %s <thread_count>\n", prog_name);
   exit(0);
}  /* Usage */

void Read_matrix(char* prompt, double A[], int m, int n) {
   int             i, j;

   printf("%s\n", prompt);
   for (i = 0; i < m; i++) 
      for (j = 0; j < n; j++)
         scanf("%lf", &A[i*n+j]);
}  /* Read_matrix */


/*------------------------------------------------------------------
 * Function:        Read_vector
 * Purpose:         Read in the vector x
 * In arg:          prompt, n
 * Out arg:         x
 */
void Read_vector(char* prompt, double x[], int n) {
   int   i;

   printf("%s\n", prompt);
   for (i = 0; i < n; i++) 
      scanf("%lf", &x[i]);
}  /* Read_vector */

void *mat_vect(void*rank)
{
   long my_rank = (long) rank;
   int i, j;
   int local_m = m/CORE_NUMBER; 
   int my_first_row = my_rank*local_m;
   int my_last_row = (my_rank+1)*local_m - 1;

   for (i = my_first_row; i <= my_last_row; i++) {
      y[i] = 0.0;
      for (j = 0; j < n; j++)
          y[i] += A[i*n+j]*x[j];
   }

   return NULL;
}  
