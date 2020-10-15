#include "pfcswitch.h"
#include "defs.h"
#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void log_switch(char* to_log)
{
    FILE* f = fopen("../log/switch.log", "a");
    fprintf(f, "%s\n", to_log);
    fclose(f);
}

bool pfc1_failed = false;
bool pfc2_failed = false;
bool pfc3_failed = false;
bool hold_on = true;

void sigerr1()
{
    pfc1_failed = true;
}

void sigerr2()
{
    pfc2_failed = true;
}

void sigerr3()
{
    pfc3_failed = true;
}

void sigemr()
{
    char to_log[128];
    char ts[64];
    timestamp(ts);
    sprintf(to_log, "[%s] Emergency abort", ts);
    
    log_switch(to_log);
    hold_on = false;
}

int check_status(int pid)
{
    int status;
    return waitpid(pid, &status, WNOHANG);
}

void pfcswitch(int pid_pfc1, int pid_pfc2, int pid_pfc3)
{
    signal(SIGERR1, sigerr1);
    signal(SIGERR2, sigerr2);
    signal(SIGERR3, sigerr3);
    signal(SIGEMR,  sigemr);
    char to_log[256];
    char ts[64];
    while(hold_on)
    {
        timestamp(ts);
        
        if(pfc1_failed)
        {
            int status = check_status(pid_pfc1);
            sprintf(to_log, "[%s] %s error. Process is in state: %s", ts, "PFC_1", status == STATUS_ERROR ? "STATUS_ERROR":
                                                                           status == STATUS_OK    ? "STATUS_OK" : "STATUS_EXIT");
            log_switch(to_log);
            
            pfc1_failed = false;
        }
        if(pfc2_failed)
        {
            int status = check_status(pid_pfc2);
            sprintf(to_log, "[%s] %s error. Process is in state: %s", ts, "PFC_2", status == STATUS_ERROR ? "STATUS_ERROR":
                                                                           status == STATUS_OK    ? "STATUS_OK" : "STATUS_EXIT");
            log_switch(to_log);
            pfc2_failed = false;
        }
        if(pfc3_failed)
        {
            int status = check_status(pid_pfc1);
            sprintf(to_log, "[%s] %s error. Process is in state: %s", ts, "PFC_3", status == STATUS_ERROR ? "STATUS_ERROR":
                                                                           status == STATUS_OK    ? "STATUS_OK" : "STATUS_EXIT");
            log_switch(to_log);
            pfc2_failed = false;
        }
        
        usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
    }
}
