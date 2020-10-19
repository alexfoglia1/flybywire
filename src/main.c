#include "defs.h"
#include "genfail.h"
#include "pfc.h"
#include "pfcswitch.h"
#include "transducers.h"
#include "wes.h"
#include "utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

void init_temporary()
{    
    FILE* f = fopen("../tmp/shared.tmp", "w");
    fclose(f);
    mkfifo("../tmp/pipe", S_IRWXU);
    
    FILE* fr = fopen("../src/map.html", "r");
    FILE* fw = fopen("map.html", "w");
    
    char ch;
    while((ch = fgetc(fr)) != EOF)
    {
        fputc(ch, fw);
    }
    fclose(fr);
    fclose(fw);
}

void rem_temporary()
{
    unlink("../tmp/shared.tmp");
    unlink("../tmp/pipe");
}

void clear_logs()
{
    remove("../log/speedPFC1.log");
    remove("../log/speedPFC2.log");
    remove("../log/speedPFC3.log");
    remove("../log/expectedPFC1.log");
    remove("../log/expectedPFC2.log");
    remove("../log/expectedPFC3.log");
    remove("../log/failures.log");
    remove("../log/status.log");
    remove("../log/switch.log");
}

int sock_bind(int sock_fd)
{
    struct sockaddr_in saddr;
    memset(&saddr, 0x00, sizeof(struct sockaddr_in));
    
    saddr.sin_family      = AF_INET;
    saddr.sin_port        = htons(PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;
    
    return bind(sock_fd, (struct sockaddr*) &saddr, sizeof(struct sockaddr));
}

int main()
{
    int can_exit;
    clear_logs();
    init_temporary();
    int tnd_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock_bind(tnd_socket) != 0)
    {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }
    set_tnd_sock_fd(&tnd_socket);
    
    int pid[N_SUBPROCESSES];
    pid[0] = fork();
    if(pid[0] == 0)
    {
        /* Child process  TND_1 */
        transducers_loop(TND_1);
        exit(EXIT_SUCCESS);
    }
    pid[1] = fork();
    if(pid[1] == 0)
    {
        /* Child process  TND_2 */
        transducers_loop(TND_2);
        exit(EXIT_SUCCESS);
    }
    pid[2] = fork();
    if(pid[2] == 0)
    {
        /* Child process  TND_3 */
        transducers_loop(TND_3);
        exit(EXIT_SUCCESS);
    }
      
    pid[3] = fork();
    if(pid[3] == 0)
    {
        /* Child process  PFC_1 */
        pfc_loop(PFC_1);
        exit(EXIT_SUCCESS);

    }
    pid[4] = fork();
    if(pid[4] == 0)
    {
        /* Child process  PFC_2 */
        pfc_loop(PFC_2);
        exit(EXIT_SUCCESS);
    }
    pid[5] = fork();
    if(pid[5] == 0)
    {
        /* Child process PFC_3 */
        pfc_loop(PFC_3);
        exit(EXIT_SUCCESS);
    }
    
    pid[6] = fork();
    if(pid[6] == 0)
    {
        /* Child process GEN_FAILURE */
        generate_failures(pid[3], pid[4], pid[5]);
        /* exit on SIGKILL from parent */
    }
    
    pid[7] = fork();
    if(pid[7] == 0)
    {
        /* Child process PFC Disconnect Switch */
        pfcswitch(pid[3], pid[4], pid[5]);
        
        /* exit on emergency message from WES */
        for(int i = 0; i < N_SUBPROCESSES; i++)
        {
            kill(pid[i], SIGKILL);
        }
        exit(EXIT_FAILURE);
    }
    
    pid[8] = fork();
    if(pid[8] == 0)
    {
        /* Child process WES */
        check(pid[7]);
    }
    
    for(int i = 0; i < N_SUBPROCESSES - 3; i++)
    {
        wait(&can_exit);
    }
    
    /* cleanup */
    close(tnd_socket);
    rem_temporary();
    kill(pid[6] /* GEN_FAILURE */, SIGKILL);
    kill(pid[7] /* PFC Disconnect Switch */, SIGKILL);
    kill(pid[8] /* WES */, SIGKILL);
    
    return 0;
}


