#include "transducers.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <unistd.h>

void log_speed(const char* filename, float speed_m_s)
{
    FILE* f = fopen(filename, "a");
    fprintf(f, "%f\n", speed_m_s);
    fclose(f);
}

void transducers_loop(int input_fd, tnd_id id)
{
    const char* filename = (id == TND_1) ? "../log/speedPFC1.log" :
                           (id == TND_2) ? "../log/speedPFC2.log" :
                           "../log/speedPFC3.log";
    
    unsigned int expected_cnt = 0;                      
    pfc_message in_msg;
    int bytes = 0;
    while(true)
    {
        usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
        if(id == TND_1)
        {
            struct sockaddr_in cliaddr;
            int len;
            bytes = recvfrom(input_fd, (char*)&in_msg, sizeof(pfc_message), 0, (struct sockaddr*) &cliaddr, &len);
        }
        else if(id == TND_2)
        {
            int pipe_fd = open("../tmp/pipe", O_RDONLY);
            while(bytes < sizeof(pfc_message))
            {
                bytes = read(pipe_fd, &in_msg, sizeof(pfc_message));
            }
            bytes = 0;
            close(pipe_fd);
        }
        else
        {
            /* id == TND_3 */
            FILE* f = fopen("../tmp/shared.tmp", "r");
            fseek(f, expected_cnt * sizeof(pfc_message), SEEK_CUR);
            bytes = sizeof(pfc_message) * fread(&in_msg, sizeof(pfc_message), 1, f);
            fclose(f);
            
            expected_cnt = in_msg.cnt + 1;
        }
        if(in_msg.end_flag == 0)
        {
            log_speed(filename, in_msg.speed_m_s);
        }
        else if(in_msg.end_flag == 1)
        {
            return;
        }
    }
}
