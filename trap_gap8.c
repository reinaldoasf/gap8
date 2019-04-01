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


#define FC_FREQ       (300000000)
#define CLUSTER_FREQ  (200000000)
#define F_DIV         (1000000)
#define NPOINTS       (10000000)
//#define NUM_THREADS   (8)
#define CORE_NUMBER   (8)
#define DATA_MAX      (10)

struct integral
{
	int a;
	int b;
	int n;
	int thread_count;
};


uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}


void Master_Entry(void* arg)
{
    CLUSTER_CoresFork(Trap, arg);
}


int f(int x) {
   int return_val;

   return_val = x*x;
   return return_val;
}  /* f */


int approx;

int Trap(void*arg){
   int  i;
	integral prov = (integral) arg;
   prov.h = (prov.b-prov.a)/prov.n; 
   approx = (f(prov.a) + f(prov.b))/2.0; 
//#  pragma omp parallel for num_threads(thread_count) \
//      reduction(+: approx)
	EU_MutexLock(0);
   for (i = 1; i <= prov.n-1; i++)
     approx += f(prov.a + i*prov.h);
	EU_MutexUnlock(0);
   approx = prov.h*approx; 

   return approx;
}  /* Trap */

int main()
{
	   int  global_result = 0.0; /* Store result in global_result */
   int a=0, b=100000000000;                 /* Left and right endpoints*/
   int     n=3000000;                    /* Total number of trapezoid*/
   int     thread_count=CORE_NUMBER;
	integral integ;
	integ.a=a;
	integ.b=b;
	integ.n=n;
	integ.thread_count=thread_count;
	CLUSTER_Start(0, CORE_NUMBER);

    if (FLL_SetFrequency(uFLL_CLUSTER, CLUSTER_FREQ, 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    }
	CLUSTER_SendTask(0, Master_Entry, (void*)integ, 0);
    printf("Waiting...\n");


	CLUSTER_Wait(0);


   printf("With n = %d trapezoids, our estimate\n", integ.n);
   printf("of the integral from %f to %f = %.14e\n",
      integ.a, integ.b, approx);
   return 0;

}
