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
#include <unistd.h>

bool alterate_next = false;
void sigusr1()
{
    alterate_next = true;
}

utc_timestamp parse_lat_lon(const char* nmea_string, float* p_lat_lon)
{
    utc_timestamp utc_time;
    int nmea_time;
    char ignore[NMEA_SIZE];
    
    sscanf(nmea_string, "$GPGLL,%f,N,%f,E,%d,%s", &p_lat_lon[0], &p_lat_lon[1], &nmea_time, ignore);
    utc_time.hour = 10*extract_digit_from(nmea_time, DIGIT_0) + extract_digit_from(nmea_time, DIGIT_1);
    utc_time.min  = 10*extract_digit_from(nmea_time, DIGIT_2) + extract_digit_from(nmea_time, DIGIT_3);
    utc_time.sec  = 10*extract_digit_from(nmea_time, DIGIT_4) + extract_digit_from(nmea_time, DIGIT_5);
    
    return utc_time;
}

void send_pfc_msg(pfc_message out_msg, int output_fd, struct sockaddr_in* saddr)
{
    if(out_msg.id == PFC_1)
    {
        sendto(output_fd, (char*) &out_msg, sizeof(pfc_message), 0, (const struct sockaddr*) saddr, sizeof(struct sockaddr));
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

void pfc_loop(int output_fd, pfc_id id, struct sockaddr_in* saddr)
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
    
    FILE* nmea_file = fopen("G18.txt", "r");

    while(fgets(act_nmea, NMEA_SIZE, nmea_file) != NULL)
    {
        if(string_starts_with("$GPGLL", (const char*) act_nmea))
        {
            act_t = parse_lat_lon((const char*) act_nmea, act_lat_lon);
            float act_t_s = utc_to_seconds(act_t);
            float old_t_s = utc_to_seconds(old_t);
            float dt_s = (act_t_s) - (old_t_s);
            float distance_m = distance_ll(old_lat_lon[0], old_lat_lon[1], act_lat_lon[0], act_lat_lon[1]);
                
            out_msg.speed_m_s = (distance_m > 0 && dt_s > 0) ? distance_m / dt_s : 0;
            if(alterate_next)
            {
                out_msg.speed_m_s = (float) ((int)round(out_msg.speed_m_s) >> 2);
                alterate_next = false;
            }
            out_msg.end_flag = 0;
            out_msg.cnt = processed_nmea;
            
            char filename[64];
            sprintf(filename, "../log/expectedPFC%d.log", id + 1);
            FILE*log = fopen(filename, "a");
            fprintf(log, "%f\n", out_msg.speed_m_s);
            fclose(log);
            
            send_pfc_msg(out_msg, output_fd, saddr);
         
            
            printf("[%d]\t[%d:%d:%d]\tlat: %f\tlon: %f\tspeed: %f m/s\n", processed_nmea, act_t.hour, act_t.min, act_t.sec, act_lat_lon[0], act_lat_lon[1], out_msg.speed_m_s);
            
            memcpy(old_lat_lon, act_lat_lon, 2*sizeof(float));
            memcpy(&old_t, &act_t, sizeof(utc_timestamp));
            
            usleep(1000000 * DELAY_S);
            
            processed_nmea += 1;
        }
    }
    
    pfc_message kill_msg;
    kill_msg.id = id;
    kill_msg.speed_m_s = 0.0;
    kill_msg.end_flag = 1;
    
    send_pfc_msg(kill_msg, output_fd, saddr);
}
