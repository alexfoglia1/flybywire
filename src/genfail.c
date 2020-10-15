#include "genfail.h"
#include "defs.h"
#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


float unif_01()
{
    float rnd = (float) rand();
    float rnd_max = (float) RAND_MAX;
    return rnd / rnd_max;
}

int unif_02()
{
    return (int)(3 * unif_01());
}

bool bin_test(float p_success)
{
    float r = unif_01();
    return r <= p_success;
}

void generate_failures(int pid_pfc1, int pid_pfc2, int pid_pfc3)
{
     usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
     srand((unsigned int) time(NULL));

     const int pid_pfc_vec[3] = {pid_pfc1, pid_pfc2, pid_pfc3};
     char ts[64];
     while(true)
     {
        timestamp(ts);
        
        int rand_02 = unif_02() % 3;
        int target_pid = pid_pfc_vec[rand_02];
        bool sigstop = bin_test(P_SIGSTOP);
        bool sigint  = bin_test(P_SIGINT);
        bool sigcont = bin_test(P_SIGCONT);
        bool sigusr1 = bin_test(P_SIGUSR1);
        
        FILE* f = fopen("../log/failures.log", "a");
        if(sigstop)
        {
            fprintf(f, "[%s] Send SIGSTOP to pfc %d\n", ts, 1 + rand_02);
            kill(target_pid, SIGSTOP);
        }
        if(sigint)
        {
            fprintf(f, "[%s] send SIGINT to pfc %d\n", ts, 1 + rand_02);
            kill(target_pid, SIGINT);
        }
        if(sigcont)
        {
            fprintf(f, "[%s] send SIGCONT to pfc %d\n", ts, 1 + rand_02);
            kill(target_pid, SIGCONT);
        }
        if(sigusr1)
        {
            fprintf(f, "[%s] send SIGUSR1 signal to pfc %d\n", ts, 1 + rand_02);
            kill(target_pid, SIGUSR1);
        }
        fclose(f);
        
        usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
     }
}
