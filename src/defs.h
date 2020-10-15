#ifndef DEFS_H
#define DEFS_H

#define N_SUBPROCESSES 7
#define PORT      1234

#define DELAY_S   1
#define NMEA_SIZE 44
#define NMET_SIZE 6

#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MIN  60

#define DIGIT_0   0
#define DIGIT_1   1
#define DIGIT_2   2
#define DIGIT_3   3
#define DIGIT_4   4
#define DIGIT_5   5

#define DEG_TO_RADIANS(deg) ( (180.0*deg) / M_PI)
#define EARTH_RADIUS(lat1,lon1,lat2,lon2) (sqrt((pow(pow(6378137,2) * cos(lat1),2) + pow(pow(6356752,2) * sin(lon1),2)) / (pow(6378137 * cos(lat2),2) + pow(6356752 * sin(lon2),2))))

#define P_SIGSTOP 1e-2
#define P_SIGINT  1e-4
#define P_SIGCONT 1e-1
#define P_SIGUSR1 1e-1

typedef struct
{
    unsigned int hour;
    unsigned int min;
    unsigned int sec;
} utc_timestamp;

typedef enum
{
    PFC_1, PFC_2, PFC_3
} pfc_id;

typedef enum
{
    TND_1, TND_2, TND_3
} tnd_id;


typedef struct
{
    float  speed_m_s;
    pfc_id id;
    unsigned int cnt;
    
    int end_flag;
} pfc_message;

#endif