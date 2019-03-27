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

void pi(void *arg){
  float random1, random2;
  int i, *hit_pointer, count=0;
  unsigned int seed; 

  hit_pointer = (int *) arg;
  seed = (unsigned int) FLL_GetFrequency(uFLL_SOC);

  for(i = 0; i < (NPOINTS/NUM_THREADS); i++){
    random1 = (float)(2*rand_r(&seed))/RAND_MAX;
    random2 = (float)(2*rand_r(&seed))/RAND_MAX;
    if(random1*random1+random2*random2 <= 1){
      count++;
    }
  }
  printf("%d - %d - %d\n",__core_ID(), seed, i);
  hit_pointer[__core_ID()] = count;
}

void Master_Entry(int * L1_mem)
{
    CLUSTER_CoresFork(pi, (void *) L1_mem);
}

int main(){
    float random1, random2;
    int i, count=0;

    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 0);

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);

    int *L1_mem = L1_Malloc(8);

    if (FLL_SetFrequency(uFLL_CLUSTER, CLUSTER_FREQ, 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    }

    printf("FC Frequency: %d MHz - Cluster Frequency: %d MHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV, FLL_GetFrequency(uFLL_CLUSTER)/F_DIV, current_voltage());

    CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
    printf("Waiting...\n");

    CLUSTER_Wait(0);

    for (int i = 0; i < 8; i++) {
       count += L1_mem[i];
       printf("CORE %d - count = %d\n", i, L1_mem[i]);
    }

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    printf("DONE!\n");
    printf("Pi: %d\n", (int) (100000*((float)(4*count)/NPOINTS)));
    exit(0);
}
