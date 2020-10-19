#include "pfcswitch.h"
#include "defs.h"
#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <unistd.h>

void log_switch(char* to_log)
{
    FILE* f = fopen("../log/switch.log", "a");
    fprintf(f, "%s\n", to_log);
    fclose(f);
}

int check_status(int pid)
{
    int status;
    return waitpid(pid, &status, WNOHANG);
}

void pfcswitch(int pid_pfc1, int pid_pfc2, int pid_pfc3)
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int len;
    struct sockaddr_in saddr, cliaddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons(PORT + 1);
    saddr.sin_addr.s_addr = INADDR_ANY;
    
    wes_to_switch_message in_msg;
    
    if(bind(sock_fd, (struct sockaddr*) &saddr, sizeof(struct sockaddr)) != 0)
    {
        perror("Cannot create PFC Disconnect Switch socket");
        exit(EXIT_FAILURE);
    }
    
    bool hold_on = true;
    char to_log[256];
    char ts[64];
    while(hold_on)
    {
        size_t recv_bytes = 0;
        while(recv_bytes < sizeof(wes_to_switch_message))
        {
            recv_bytes = recvfrom(sock_fd, (char*)&in_msg, sizeof(wes_to_switch_message), 0, (struct sockaddr*) &cliaddr, &len);
        }
        timestamp(ts);
        

        if(in_msg.pfc1_failed)
        {
            int status = check_status(pid_pfc1);
            sprintf(to_log, "[%s] %s error. Process is in state: %s", ts, "PFC_1", status == STATUS_ERROR ? "STATUS_ERROR":
                                                                           status == STATUS_OK    ? "STATUS_OK" : "STATUS_EXIT");
            log_switch(to_log);
        }
        if(in_msg.pfc2_failed)
        {
            int status = check_status(pid_pfc2);
            sprintf(to_log, "[%s] %s error. Process is in state: %s", ts, "PFC_2", status == STATUS_ERROR ? "STATUS_ERROR":
                                                                           status == STATUS_OK    ? "STATUS_OK" : "STATUS_EXIT");
            log_switch(to_log);
        }
        if(in_msg.pfc3_failed)
        {
            int status = check_status(pid_pfc1);
            sprintf(to_log, "[%s] %s error. Process is in state: %s", ts, "PFC_3", status == STATUS_ERROR ? "STATUS_ERROR":
                                                                           status == STATUS_OK    ? "STATUS_OK" : "STATUS_EXIT");
            log_switch(to_log);
        }
        
        if(in_msg.pfc1_failed && in_msg.pfc2_failed && in_msg.pfc3_failed)
        {
            sprintf(to_log, "[%s] Emergency abort", ts);

            log_switch(to_log);
            hold_on = false;
        }
    }
    
    close(sock_fd);
}
