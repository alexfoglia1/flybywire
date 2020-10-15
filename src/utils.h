#ifndef UTILS_H
#define UTILS_H

#include "defs.h"
#include <stdbool.h>

float utc_to_seconds(utc_timestamp t);
bool  string_starts_with(const char *prefix, const char *string);
int   extract_digit_from(int n, int pos);
float distance_ll(float o_lat, float o_lon, float p_lat, float p_lon);
void timestamp(char* dst);

#endif //UTILS_H
