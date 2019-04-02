//* Compile: gcc -g -Wall -fopenmp -o omp_trap3 omp_trap3.c
#include "cmsis.h"
#include <math.h>
#include "gap_common.h"
#include "mbed_wait_api.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>
#include <time.h>


#define FC_FREQ       (250000000)
#define CLUSTER_FREQ  (150000000)
#define F_DIV         (1000000)
#define NPOINTS       (10000000)
//#define NUM_THREADS   (8)
#define CORE_NUMBER   (8)
#define DATA_MAX      (10)

typedef struct integral
{
	int a;
	int b;
	int n;
	int thread_count;
}integral;

uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}



int f(int x) {
   int return_val;

   return_val = x*x;
   return return_val;
}  /* f */


int approx;
void Trap(void* arg){
   int  i;
   int h;
	integral* prov = (integral*) arg;
    h = (prov[__core_ID()].b-prov[__core_ID()].a)/prov[__core_ID()].n; 
   //printf("o valor de a = %d, o valor de b = %d, o valor de n = %d no core %d",&prov[__core_ID()]->a,&prov[__core_ID()]->b,&prov[__core_ID()]->n,__core_ID()); 
   approx = (f(prov[__core_ID()].a) + f(prov[__core_ID()].b))/2.0; 
//#  pragma omp parallel for num_threads(thread_count)      reduction(+: approx)
	EU_MutexLock(0);
   for (i = 1; i <= prov[__core_ID()].n-1; i++)
     approx += f(prov[__core_ID()].a + i*h);
	EU_MutexUnlock(0);
   approx = h*approx; 

//   return approx;
}  /* Trap */
void Master_Entry(void* arg) 
{
	CLUSTER_CoresFork(Trap, arg);
}


int main()
{
	   int  global_result = 0.0; /* Store result in global_result */
   int a=0, b=1000;                 /* Left and right endpoints*/
   int     n=20;                    /* Total number of trapezoid*/
   int     thread_count=CORE_NUMBER;
	integral* integ=L1_Malloc(CORE_NUMBER*sizeof(struct integral));

	integ->a=a;
	integ->b=b;
	printf("o valor de b e: %d\n\n",&integ->b);
	integ->n=n;
	integ->a=a;
	CLUSTER_Start(0, CORE_NUMBER);

    if (FLL_SetFrequency(uFLL_CLUSTER, CLUSTER_FREQ, 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    }
	CLUSTER_SendTask(0, Master_Entry,(void*) integ, 0);
    printf("Waiting...\n");


	CLUSTER_Wait(0);


   printf("With n = %d trapezoids, our estimate\n", integ->n);
   printf("of the integral from %d to %d = \n",
      integ->a, integ->b, approx);
   return 0;
}
