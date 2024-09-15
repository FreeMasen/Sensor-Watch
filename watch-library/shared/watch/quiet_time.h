#ifndef _QUIET_TIME_H_INCLUDED
#define _QUIET_TIME_H_INCLUDED

#include "watch.h"

#define FILE_NAME "quite_time.bin"

#define QUIET_TIME_DAY_U 1
#define QUIET_TIME_DAY_M 2
#define QUIET_TIME_DAY_T 4
#define QUIET_TIME_DAY_W 8
#define QUIET_TIME_DAY_R 16
#define QUIET_TIME_DAY_F 32
#define QUIET_TIME_DAY_S 64
#define QUIET_TIME_DAY_WD (QUIET_TIME_DAY_M | QUIET_TIME_DAY_T | QUIET_TIME_DAY_W | QUIET_TIME_DAY_R | QUIET_TIME_DAY_F)
#define QUIET_TIME_DAY_WE (QUIET_TIME_DAY_U | QUIET_TIME_DAY_S)
#define QUIET_TIME_DAY_ALL (QUIET_TIME_DAY_WD | QUIET_TIME_DAY_WE)

/**
 * ISO day resolution
 */
#define QUIET_TIME_DAY_IDX {QUIET_TIME_DAY_M, QUIET_TIME_DAY_T,                   \
                            QUIET_TIME_DAY_W, QUIET_TIME_DAY_R, QUIET_TIME_DAY_F, \
                            QUIET_TIME_DAY_S, QUIET_TIME_DAY_U, 0}

typedef struct
{
    // 0-23
    uint8_t hour : 5;
    // 0:0,1:15,2:30,3:45
    uint8_t minute : 2;
} quiet_time_t;

typedef union
{
    struct {
        quiet_time_t start;
        quiet_time_t end;
        uint8_t days : 2;
    } unit;
    uint16_t reg;
} quite_time_save2_t;

typedef struct
{
    quiet_time_t start;
    quiet_time_t end;
    uint8_t days;
} quiet_time_save_t;

void quiet_time_save_data(quiet_time_save_t *state);
void quiet_time_load_data(quiet_time_save_t *state);
bool quiet_time_is_between(quiet_time_save_t *state, watch_date_time *date_time);
#endif
