#include "quiet_time.h"
#include "filesystem.h"
#include "watch_utility.h"

void quiet_time_save_data(quiet_time_save_t *state)
{
    filesystem_write_file(FILE_NAME, (char *)state, sizeof(state));
}

void quiet_time_load_data(quiet_time_save_t *state)
{
    int32_t fsize = filesystem_get_file_size(FILE_NAME);
    if (fsize != sizeof(quiet_time_save_t))
    {
        state->start.hour = 2;
        state->start.minute = 0;
        state->end.hour = 8;
        state->end.minute = 0;
        printf("Defaulted start %02d:%02d\n", state->start.hour, state->start.minute);
        printf("Defaulted end %02d:%02d\n", state->end.hour, state->end.minute);
        quiet_time_save_data(state);
        return;
    }
    filesystem_read_file(FILE_NAME, (char *)state, sizeof(quiet_time_save_t));
}

bool quiet_time_is_between(quiet_time_save_t *state, watch_date_time *date_time)
{
    static const uint8_t weekdays[7] = {
        QUIET_TIME_DAY_M, QUIET_TIME_DAY_T,
        QUIET_TIME_DAY_W, QUIET_TIME_DAY_R,
        QUIET_TIME_DAY_F, QUIET_TIME_DAY_S,
        QUIET_TIME_DAY_U};
    uint8_t day = weekdays[watch_utility_get_iso8601_weekday_number(date_time->unit.year + WATCH_RTC_REFERENCE_YEAR, date_time->unit.month, date_time->unit.day) - 1];
    // If weekday isn't in the days bitmask, it is not quiet time
    if ((state->days & day) <= 0)
    {
        return false;
    }
    // If start hour is current hour, compare minutes
    if  (date_time->unit.hour == state->start.hour)
    {
        return date_time->unit.minute >= state->start.minute;
    }
    // if end hour is current hour, compare minutes
    if (date_time->unit.hour == state->end.hour)
    {
        return date_time->unit.minute < state->start.minute;
    }
    // Otherwise hour only calculation is needed
    return date_time->unit.hour >= state->start.hour && date_time->unit.hour < state->end.hour;
}
