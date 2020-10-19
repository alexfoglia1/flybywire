#include "transducers.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <unistd.h>

int* p_sock_fd;

void set_tnd_sock_fd(int* _p_sock_fd)
{
    p_sock_fd = _p_sock_fd;
}

void log_speed(const char* filename, float speed_m_s)
{
    FILE* f = fopen(filename, "a");
    fprintf(f, "%f\n", speed_m_s);
    fclose(f);
}


void transducers_loop(tnd_id id)
{
    const char* filename = (id == TND_1) ? "../log/speedPFC1.log":
                           (id == TND_2) ? "../log/speedPFC2.log":
                                           "../log/speedPFC3.log";
    
    unsigned int expected_cnt = 1;                      
    pfc_message in_msg;
    int in_bytes = 0;
    while(true)
    {
        if(id == TND_1)
        {
            struct sockaddr_in cliaddr;
            int len;
            in_bytes = recvfrom(*p_sock_fd, (char*)&in_msg, sizeof(pfc_message), 0, (struct sockaddr*) &cliaddr, &len);
        }
        else if(id == TND_2)
        {
            int pipe_fd = open("../tmp/pipe", O_RDONLY);
            while(in_bytes < sizeof(pfc_message))
            {
                in_bytes = read(pipe_fd, &in_msg, sizeof(pfc_message));
            }
            in_bytes = 0;
            close(pipe_fd);
        }
        else
        {
            /* id == TND_3 */
            FILE* f = fopen("../tmp/shared.tmp", "r");
            
            int act_cnt = 0;
            do
            {
                in_bytes = sizeof(pfc_message) * fread(&in_msg, sizeof(pfc_message), 1, f);
                if(in_bytes < sizeof(pfc_message))
                {
                    fclose(f);
                    usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
                    f = fopen("../tmp/shared.tmp", "r");
                }
                else
                {
                    act_cnt = in_msg.cnt;
                }
            }while(act_cnt < expected_cnt);

            fclose(f);
        }
        if(in_msg.end_flag == 0)
        {
            log_speed(filename, in_msg.speed_m_s);
            expected_cnt += 1;
        }
        else
        {
            return;
        }
        
        usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
    }
}
