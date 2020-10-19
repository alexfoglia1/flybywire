#include "wes.h"
#include "defs.h"
#include "utils.h"

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int expected_cnt = 1;

float read_last(const char* filename)
{
    char *line = NULL;
    size_t  line_len = 0;
    ssize_t byte_len = 0;
    float read_speed = 0;
    int   read_cnt   = 0;
    
    FILE* fp = fopen(filename, "r");
    for(int i = 0; i < expected_cnt; i++)
    {
        byte_len = getline(&line, &line_len, fp);
    }
    fclose(fp);
    
    return atof(line);
}

void check(int pid_dis)
{
    usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
    
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons(PORT + 1);
    saddr.sin_addr.s_addr = INADDR_ANY;
    
    wes_to_switch_message out_msg;
    char ts[64];
    while(true)
    {
        timestamp(ts);
        
        memset(&out_msg, 0x00, sizeof(wes_to_switch_message));
        float v1, v2, v3;
        v1 = read_last("../log/speedPFC1.log");
        v2 = read_last("../log/speedPFC2.log");
        v3 = read_last("../log/speedPFC3.log");
        expected_cnt += 1;
        
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

                out_msg.pfc1_failed = true;
                out_msg.pfc2_failed = true;
                out_msg.pfc3_failed = true;
            }
            else
            {
                printf("[%s] WES error pfc", ts);
                fprintf(f, "[%s] WES error pfc", ts);
                if(v1 == v2)
                {
                    printf("3");
                    fprintf(f, "3");

                    out_msg.pfc3_failed = true;
                }
                else if(v1 == v3)
                {
                    printf("2");
                    fprintf(f, "2");

                    out_msg.pfc2_failed = true;
                }
                else
                {
                    printf("1");
                    fprintf(f, "1");

                    out_msg.pfc1_failed = true;
                }

            }
        }
        printf(":\tpfc1(%f), pfc2(%f), pfc3(%f)\n", v1, v2, v3);
        fprintf(f, ":\tpfc1(%f), pfc2(%f), pfc3(%f)\n", v1, v2, v3);
        fclose(f);
        
        sendto(sock_fd, (char*) &out_msg, sizeof(wes_to_switch_message), 0, (const struct sockaddr*) &saddr, sizeof(struct sockaddr));
    
        usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
    }    
}
