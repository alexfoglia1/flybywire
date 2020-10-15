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
    float earth_radius_m = EARTH_RADIUS(o_lat, o_lon, p_lat, p_lon);
    float delta_lat = p_lat - o_lat;
    float delta_lon = p_lon - o_lon;

    float a = sin(delta_lat/2) * sin(delta_lat/2) + sin(delta_lon/2) * sin(delta_lon/2) * cos(DEG_TO_RADIANS(o_lat)) * cos(DEG_TO_RADIANS(p_lat));
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    
    return earth_radius_m * c;
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
