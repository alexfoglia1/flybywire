#include "utils.h"
#include "defs.h" 

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

float utc_to_seconds(utc_timestamp t)
{
    return t.hour * SECONDS_IN_HOUR + t.min * SECONDS_IN_MIN + t.sec;
}

bool string_starts_with(const char *prefix, const char *string)
{
    size_t lenpre = strlen(prefix);
    size_t lenstr = strlen(string);
    return lenstr < lenpre ? false : memcmp(prefix, string, lenpre) == 0;
}

int extract_digit_from(int n, int pos)
{
    char number[NMET_SIZE];
    sprintf(number, "%d", n);
    int digit = (int) ( number[pos] - '0' );
    
    return digit;
}

float distance_ll(float o_lat, float o_lon, float p_lat, float p_lon)
{
    float R = 6371e3;
    float phi1 = o_lat;
    float phi2 = p_lat;
    float dphi = phi2 - phi1;
    float dlam = p_lon - o_lon; 

    float a = sin(dphi/2) * sin(dphi/2) + cos(phi1) * cos(phi2) * sin(dlam/2) * sin(dlam/2);
    float c = 2 * atan2(sqrt(a), sqrt(1-a));

    return R * c;
}

void timestamp(char* dst)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    char* ts = ctime((time_t*) &tv);
    int end = strlen(ts) - 1;
    ts[end] = '\0';
    
    sprintf(dst, "%s", ts);
}
