#ifndef DEFS_H
#define DEFS_H

#define N_SUBPROCESSES 9
#define PORT      1234

#define DELAY_S   0.01
#define NMEA_SIZE 44
#define NMET_SIZE 6
#define FPRINTED_FLOAT_SIZE 9

#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MIN  60
#define SECONDS_TO_MICROSECONDS(usec) (1000000 * usec)

#define DIGIT_0   0
#define DIGIT_1   1
#define DIGIT_2   2
#define DIGIT_3   3
#define DIGIT_4   4
#define DIGIT_5   5

#define DEG_TO_RADIANS(deg) ( (M_PI*deg) / 180.0)

#define P_SIGSTOP 1e-200
#define P_SIGINT  1e-400
#define P_SIGCONT 1e-100
#define P_SIGUSR1 1e-100

#define STATUS_ERROR -1
#define STATUS_OK     0

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
    unsigned int cnt;
    pfc_id id;
    
    unsigned int end_flag;
} pfc_message;

typedef struct
{
    unsigned int pfc1_failed;
    unsigned int pfc2_failed;
    unsigned int pfc3_failed;
} wes_to_switch_message;

#endif //DEFS_H
