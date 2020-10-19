#include "pfc.h"
#include "defs.h"
#include "utils.h"

#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <unistd.h>

bool alterate_next = false;
void sigusr1()
{
    alterate_next = true;
}

utc_timestamp parse_gpgll(const char* nmea_string, float* p_lat_lon)
{
    utc_timestamp utc_time;
    int nmea_time;
    
    char cp_in[NMEA_SIZE];
    memcpy(cp_in, nmea_string, NMEA_SIZE);
    
    char qLat[2],qLon[2];
    char nmeaLat[NMEA_SIZE];
    char nmeaLon[NMEA_SIZE];

    strtok(cp_in, ",");
    sprintf(nmeaLat, "%s", strtok(NULL, ","));
    sprintf(qLat, "%s", strtok(NULL, ","));
    sprintf(nmeaLon, "%s", strtok(NULL, ","));
    sprintf(qLon, "%s", strtok(NULL, ","));
    nmea_time = atoi(strtok(NULL, ","));
    
    char dd[3];
    char ddd[4];
    char mm_mmmm_lat[8];
    char mm_mmmm_lon[8];
    for(int i = 0; i < 2; i++)
    {
        dd[i] = nmeaLat[i];
    }
    dd[2] = '\0';
    
    for(int i = 3; i < 3 + 8; i++)
    {
        mm_mmmm_lat[i - 3] = nmeaLat[i];
    }
    mm_mmmm_lat[7] = '\0';
    
    for(int i = 0; i < 3; i++)
    {
        ddd[i] = nmeaLon[i];
    }
    ddd[3] = '\0';
    
    for(int i = 4; i < 4 + 8; i++)
    {
        mm_mmmm_lon[i - 4] = nmeaLon[i];
    }
    mm_mmmm_lon[7] = '\0';

    p_lat_lon[0] = atoi(dd)  + atof(mm_mmmm_lat)/60.0f;
    p_lat_lon[1] = atoi(ddd) + atof(mm_mmmm_lon)/60.0f;
    
    utc_time.hour = 10*extract_digit_from(nmea_time, DIGIT_0) + extract_digit_from(nmea_time, DIGIT_1);
    utc_time.min  = 10*extract_digit_from(nmea_time, DIGIT_2) + extract_digit_from(nmea_time, DIGIT_3);
    utc_time.sec  = 10*extract_digit_from(nmea_time, DIGIT_4) + extract_digit_from(nmea_time, DIGIT_5);
    
    return utc_time;
}

void send_pfc_msg(pfc_message out_msg)
{
    if(out_msg.id == PFC_1)
    {
        int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        struct sockaddr_in saddr;
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(PORT);
        saddr.sin_addr.s_addr = INADDR_ANY;
        
        sendto(sock_fd, (char*) &out_msg, sizeof(pfc_message), 0, (const struct sockaddr*) &saddr, sizeof(struct sockaddr));
        close(sock_fd);
    }
    else if(out_msg.id == PFC_2)
    {
        int pipe_fd = open("../tmp/pipe", O_WRONLY);
        write(pipe_fd, &out_msg, sizeof(pfc_message));
        close(pipe_fd);
    }
    else
    {
        FILE* f = fopen("../tmp/shared.tmp", "a");
        fwrite(&out_msg, sizeof(pfc_message), 1, f);
        fclose(f);
    }
}

void pfc_loop(pfc_id id)
{
    signal(SIGUSR1, sigusr1);
    
    char act_nmea[NMEA_SIZE];
    pfc_message out_msg;
    unsigned int processed_nmea;
    float old_lat_lon[2];
    utc_timestamp old_t;
    float act_lat_lon[2];
    utc_timestamp act_t;
    
    memset(act_nmea, 0x00, NMEA_SIZE);
    memset(&out_msg, 0x00, sizeof(pfc_message));
    memset(old_lat_lon, 0x00, 2 * sizeof(float));
    memset(act_lat_lon, 0x00, 2 * sizeof(float));
    memset(&old_t, 0x00, sizeof(utc_timestamp));
    memset(&act_t, 0x00, sizeof(utc_timestamp));
    
    processed_nmea = 0;
    out_msg.id = id;
    out_msg.end_flag = 0;
    
    FILE* nmea_file = fopen("G18.txt", "r");
    while(fgets(act_nmea, NMEA_SIZE, nmea_file) != NULL)
    {
        if(string_starts_with("$GPGLL", (const char*) act_nmea))
        {
            processed_nmea += 1;
            act_t = parse_gpgll((const char*) act_nmea, act_lat_lon);
            float act_t_s = utc_to_seconds(act_t);
            float old_t_s = utc_to_seconds(old_t);
            float dt_s    = (processed_nmea == 1) ? 0: 
                                    act_t_s - old_t_s;
            float dist_m  = (processed_nmea == 1) ? 0: 
                              distance_ll(old_lat_lon[0], old_lat_lon[1], act_lat_lon[0], act_lat_lon[1]);
            
            out_msg.speed_m_s = (dist_m > 0 && dt_s > 0) ? dist_m / dt_s : 0;
            if(alterate_next)
            {
                out_msg.speed_m_s = (float) ((int)round(out_msg.speed_m_s) >> 2);
                alterate_next = false;
            }
            
            out_msg.cnt = processed_nmea;
            
            send_pfc_msg(out_msg);

            memcpy(old_lat_lon, act_lat_lon, 2*sizeof(float));
            memcpy(&old_t, &act_t, sizeof(utc_timestamp));
            
            if(out_msg.speed_m_s > 0 && id == PFC_1)
            {
                FILE* map_file = fopen("map.html", "a");
                fprintf(map_file, "\n<script>\nL.marker([%f, %f]).addTo(mymap)\n</script>\n", act_lat_lon[0], act_lat_lon[1]);
                fclose(map_file);
            }
            usleep(SECONDS_TO_MICROSECONDS(DELAY_S));
        }
    }
    
    pfc_message kill_msg;
    kill_msg.id = id;
    kill_msg.speed_m_s = 0.0;
    kill_msg.end_flag = 1;
    kill_msg.cnt = processed_nmea + 1;
    
    send_pfc_msg(kill_msg);

}
