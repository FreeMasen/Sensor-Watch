/*
 * MIT License
 *
 * Copyright (c) 2022 Joey Castillo
 * Copyright (c) 2024 Robert Masen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include "quiet_time_face.h"
#include "watch.h"
#include "filesystem.h"
#include "watch_utility.h"
#include "watch_private_display.h"
#include "quiet_time.h"

#define SET_START 0
#define SET_END 1
#define SET_DAYS 2

typedef struct
{
    uint8_t position;
    quiet_time_save_t state;
} quiet_time_state_t;

static void increment_time(quiet_time_t *time)
{
    if (time->minute >= 3)
    {
        if (time->hour >= 23)
        {
            time->hour = 0;
        }
        else
        {
            time->hour += 1;
        }
        time->minute = 0;
    }
    else
    {
        time->minute += 1;
    }
}

static bool format_time(movement_settings_t *settings, quiet_time_t *time, char *buf)
{
    uint32_t hour;
    bool ret = false;
    if (settings->bit.clock_mode_24h || time->hour <= 12)
    {
        hour = time->hour;
    }
    else
    {
        ret = true;
        hour = time->hour - 12;
    }
    sprintf(buf, "%02d%02d", hour, time->minute * 15);
    return ret;
}

static void update_screen_value(movement_settings_t *settings, quiet_time_state_t *state)
{
    char buf[11];
    bool set_pm = false;
    watch_display_character('Q', 0);
    watch_display_character('t', 1);
    switch (state->position)
    {
    // start
    case SET_START:
    {
        printf("render set-st  %d:%d\n", state->state.start.hour, state->state.start.minute);
        watch_display_character('S', 3);
        set_pm = format_time(settings, &state->state.start, buf);
        watch_set_colon();
        break;
    }
    // end
    case SET_END:
    {
        printf("render set-end %d:%d\n", state->state.end.hour, state->state.end.minute);
        watch_display_character('E', 3);
        set_pm = format_time(settings, &state->state.end, buf);
        watch_set_colon();
        break;
    }
    // days {
    case SET_DAYS:
    {
        watch_display_character('D', 3);
        if (state->state.days == QUIET_TIME_DAY_ALL)
        {
            sprintf(buf, "ALL   ");
        }
        else if (state->state.days == QUIET_TIME_DAY_WD)
        {
            sprintf(buf, "nn-F ");
        }
        else if (state->state.days == QUIET_TIME_DAY_WE)
        {
            sprintf(buf, "S-S   ");
        }
        else
        {
            sprintf(buf, "NONE  ");
        }
        watch_clear_colon();
    }
    }
    if (set_pm)
    {
        watch_set_indicator(WATCH_INDICATOR_PM);
    }
    else
    {
        watch_clear_indicator(WATCH_INDICATOR_PM);
    }
    watch_display_string(buf, 4);
}

static void handle_increment(quiet_time_state_t *state)
{
    // printf("handle_increment %d\n", state->position);
    switch (state->position)
    {
    // start
    case SET_START:
    {
        increment_time(&state->state.start);
        break;
    }
    // end
    case SET_END:
    {
        increment_time(&state->state.end);
        break;
    }
    // days
    case SET_DAYS:
    {
        if (state->state.days == QUIET_TIME_DAY_ALL)
        {
            state->state.days = QUIET_TIME_DAY_WD;
        }
        else if (state->state.days == QUIET_TIME_DAY_WD)
        {
            state->state.days = QUIET_TIME_DAY_WE;
        }
        else if (state->state.days == QUIET_TIME_DAY_WE)
        {
            state->state.days = 0;
        }
        // 0
        else
        {
            state->state.days = QUIET_TIME_DAY_ALL;
        }
        break;
    }
    }
}

static void quite_time_face_init_time(quiet_time_t *time, int32_t default_hour)
{
    if (time->hour > 23 || time-> minute > 59)
    {
        printf("qt hour > 23 || minute > 59, resetting to %d\n", default_hour);
        time->hour = default_hour;
        time->minute = 0;
    }
}

static void quite_time_face_init_state(quiet_time_state_t *state)
{
    quiet_time_load_data(&state->state);
    quite_time_face_init_time(&state->state.start, 2);
    quite_time_face_init_time(&state->state.end, 8);
}

void quiet_time_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void **context_ptr)
{
    (void)watch_face_index;
    if (*context_ptr == NULL)
    {
        *context_ptr = malloc(sizeof(quiet_time_state_t));
        quiet_time_state_t *state = (quiet_time_state_t *)*context_ptr;
        state->position = SET_START;
        quite_time_face_init_state(state);
        return;
    }
    
}

void quiet_time_face_activate(movement_settings_t *settings, void *context)
{
    quiet_time_state_t *state = (quiet_time_state_t *)context;
    update_screen_value(settings, state);
}

bool quiet_time_face_loop(movement_event_t event, movement_settings_t *settings, void *context)
{
    quiet_time_state_t *state = (quiet_time_state_t *)context;

    switch (event.event_type)
    {
    case EVENT_TICK:
    case EVENT_ACTIVATE:

        break;
    case EVENT_LIGHT_BUTTON_DOWN:
        handle_increment(state);
        update_screen_value(settings, state);
        break;
    case EVENT_ALARM_BUTTON_UP:
        printf("change screen %d", state->position);
        switch (state->position)
        {
            case SET_START: {
                state->position = SET_END;
                break;
            }
            case SET_END: {
                state->position = SET_DAYS;
                break;
            }
            case SET_DAYS: {
                state->position = SET_START;
                break;
            }
            default: {
                printf("WARNING UNKNOWN POSITION: %d\n", state->position);
                break;
            }
        }
        printf("->%d\n", state->position);
        update_screen_value(settings, state);
        break;
    case EVENT_TIMEOUT:
        movement_move_to_face(0);
        break;
    default:
        return movement_default_loop_handler(event, settings);
    }
    return true;
}

void quiet_time_face_resign(movement_settings_t *settings, void *context)
{
    (void)settings;
    quiet_time_state_t *state = (quiet_time_state_t *)context;
    quiet_time_save_data(&state->state);
}
