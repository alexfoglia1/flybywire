#ifndef PFC_H
#define PFC_H
#include "defs.h"

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

void pfc_loop(int output_fd, pfc_id pfc, struct sockaddr_in* saddr);

#endif //PFC_H
