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

