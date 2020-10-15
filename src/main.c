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
    unlink("../tmp/pipe");
    unlink("../tmp/shared.tmp");
    mkfifo("../tmp/pipe", S_IRWXU);
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
}

int sock_bind(int sock_fd, struct sockaddr_in* saddr)
{
    memset(saddr, 0x00, sizeof(struct sockaddr_in));
    
    saddr->sin_family      = AF_INET;
    saddr->sin_port        = htons(PORT);
    saddr->sin_addr.s_addr = INADDR_ANY;
    
    return bind(sock_fd, (struct sockaddr*)saddr, sizeof(struct sockaddr));
}


int main()
{
    int can_exit;
    clear_logs();
    init_temporary();
    
    struct sockaddr_in saddr;
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock_bind(sock_fd, &saddr) != 0)
    {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }
    
    FILE* f = fopen("../tmp/shared.tmp", "w");
    fclose(f);
    
    int pid[N_SUBPROCESSES];
    pid[0] = fork();
    if(pid[0] == 0)
    {
        /* Child process  TND_1 */
        transducers_loop(sock_fd, TND_1);
        exit(EXIT_SUCCESS);
    }
    pid[1] = fork();
    if(pid[1] == 0)
    {
        /* Child process  TND_2 */
        transducers_loop(0, TND_2);
        exit(EXIT_SUCCESS);
    }
    pid[2] = fork();
    if(pid[2] == 0)
    {
        /* Child process  TND_3 */
        transducers_loop(0, TND_3);
        exit(EXIT_SUCCESS);
    }
    
    pid[3] = fork();
    if(pid[3] == 0)
    {
        /* Child process  PFC_1 */
        pfc_loop(sock_fd, PFC_1, &saddr);
        exit(EXIT_SUCCESS);

    }
    pid[4] = fork();
    if(pid[4] == 0)
    {
        /* Child process  PFC_2 */
        pfc_loop(0, PFC_2, &saddr);
        exit(EXIT_SUCCESS);
    }
    pid[5] = fork();
    if(pid[5] == 0)
    {
        /* Child process PFC_3 */
        pfc_loop(0, PFC_3, &saddr);
        exit(EXIT_SUCCESS);
    }
    
    pid[6] = fork();
    if(pid[6] == 0)
    {
        /* Child process GEN_FAILURE*/
        generate_failures(pid[3], pid[4], pid[5]);
    }
    
    for(int i = 0; i < N_SUBPROCESSES - 1; i++)
    {
        wait(&can_exit);
    }
    
    close(sock_fd);
    kill(pid[6], SIGKILL);
    unlink("../tmp/shared.tmp");
    unlink("../tmp/pipe");
    return 0;
}


