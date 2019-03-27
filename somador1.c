// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>
#include <time.h>
#define CORE_NUMBER      8
int soma=0;
//srand(time(NULL));
void Soma(int *data) {
     #include <time.h>
	for (int i=0;i<
	int a=rand();
	srand(a);
	int microsoma=0;
	//printf("Hello World from cluster core %d!\n", __core_ID());
	microsoma+=rand()%10*__core_ID();
	printf("a soma no core %d sera %d\n",__core_ID(), microsoma);
	EU_MutexLock(0);
	soma+=microsoma;
	EU_MutexUnlock(0);
}

void Master_Entry(void *arg) {
    CLUSTER_CoresFork(Soma, arg);
}

int main()
{
    printf("Fabric controller code execution for mbed_os Cluster Power On test\n");
    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);

    /* FC send a task to Cluster */
    CLUSTER_SendTask(0, Master_Entry, 0, 0);

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);
	printf("o valor total da soma eh: %d\n",soma);
    /* Check read values */
    int error = 0;

    if (error) printf("Test failed with %d errors\n", error);
    else printf("Test success\n");

    #ifdef JENKINS_TEST_FLAG
    exit(error);
    #else
    return error;
    #endif
}
