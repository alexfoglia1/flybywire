#include "wes.h"
#include "defs.h"
#include "utils.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

float read_last(const char* filename)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE* fp = fopen(filename, "r");
    while((read = getline(&line, &len, fp)) != -1);
    fclose(fp);

    return atof(line);
}

void check(int pid_dis)
{
    usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
    
    char ts[64];
    while(true)
    {
        timestamp(ts);
        
        float v1, v2, v3;
        v1 = read_last("../log/speedPFC1.log");
        v2 = read_last("../log/speedPFC2.log");
        v3 = read_last("../log/speedPFC3.log");
        
        FILE* f = fopen("../log/status.log", "a");
        if(v1 == v2 && v2 == v3)
        {
            printf("[%s] WES check OK", ts);
            fprintf(f, "[%s] WES check OK", ts);
        }
        else
        {
            bool all_neq = (v1 != v2 && v1 != v3 && v2 != v3);
            if(all_neq)
            {
                printf("[%s] WES emergency", ts);
                fprintf(f, "[%s] WES emergency", ts);
                kill(pid_dis, SIGEMR);
            }
            else
            {
                printf("[%s] WES error pfc", ts);
                fprintf(f, "[%s] WES error pfc", ts);
                if(v1 == v2)
                {
                    printf("3");
                    fprintf(f, "3");
                    kill(pid_dis, SIGERR3);
                }
                else if(v1 == v3)
                {
                    printf("2");
                    fprintf(f, "2");
                    kill(pid_dis, SIGERR2);
                }
                else
                {
                    printf("1");
                    fprintf(f, "1");
                    kill(pid_dis, SIGERR1);
                }

            }
        }
        printf(":\tpfc1(%f), pfc2(%f), pfc3(%f)\n", v1, v2, v3);
        fprintf(f, ":\tpfc1(%f), pfc2(%f), pfc3(%f)\n", v1, v2, v3);
        fclose(f);
    
        usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
    }    
}
