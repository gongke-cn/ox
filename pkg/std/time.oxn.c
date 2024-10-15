/******************************************************************************
 *                                 OX Language                                *
 *                                                                            *
 * Copyright 2024 Gong Ke                                                     *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the              *
 * "Software"), to deal in the Software without restriction, including        *
 * without limitation the rights to use, copy, modify, merge, publish,        *
 * distribute, sublicense, and/or sell copies of the Software, and to permit  *
 * persons to whom the Software is furnished to do so, subject to the         *
 * following conditions:                                                      *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included    *
 * in all copies or substantial portions of the Software.                     *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS    *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF                 *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN  *
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 ******************************************************************************/

/**
 * @file
 * Time library.
 */

#define OX_LOG_TAG "time"

#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "std.h"

/*Declaration index.*/
enum {
    ID_Time,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "Time",
    NULL
};

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/** Time value is valid.*/
#define OX_TIME_FL_TIME  (1 << 0)
/** Local date value is valid.*/
#define OX_TIME_FL_LOCAL (1 << 1)
/** UTC date value is valid.*/
#define OX_TIME_FL_UTC   (1 << 2)

/*Time.*/
typedef struct {
    int       flags;       /**< Flags.*/
    OX_Number time;        /**< Time value.*/
    struct tm local_date;  /**< Local date value.*/
    struct tm gm_date;     /**< UTC date value.*/
} OX_Time;

/*Free the time.*/
static void
time_free (OX_Context *ctxt, void *ptr)
{
    OX_Time *t = ptr;

    OX_DEL(ctxt, t);
}

/*Time's operation functions.*/
static const OX_PrivateOps
time_ops = {
    NULL,
    time_free
};

/*Time.$inf.$init*/
static OX_Result
Time_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t = NULL;
    OX_Result r;

    if (!OX_NEW(ctxt, t)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    t->flags = OX_TIME_FL_TIME;

    if (argc == 0) {
        struct timeval tv;

        if ((r = gettimeofday(&tv, NULL)) == -1) {
            r = std_system_error(ctxt, "gettimeofday");
            goto end;
        }

        t->time = (OX_Number)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    } else {
        OX_Value *ms = ox_argument(ctxt, args, argc, 0);
        OX_Number n;

        if ((r = ox_to_number(ctxt, ms, &n)) == OX_ERR)
            goto end;

        if (!isfinite(n)) {
            r = ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be a finite value"), "time");
            goto end;
        }

        t->time = n;
    }

    if ((r = ox_object_set_priv(ctxt, thiz, &time_ops, t)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    if (r == OX_ERR) {
        if (t)
            time_free(ctxt, t);
    }
    return r;
}

/*Get the time pointer.*/
static OX_Time*
get_time (OX_Context *ctxt, OX_Value *v)
{
    OX_Time *t = ox_object_get_priv(ctxt, v, &time_ops);

    if (!t)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a time object"));

    return t;
}

/*Get the time value.*/
static OX_Result
get_time_value (OX_Context *ctxt, OX_Time *t, OX_Number *n)
{
    if (!(t->flags & OX_TIME_FL_TIME)) {
        time_t time;
        OX_Number ms = fmod(t->time, 1000);

        if (t->flags & OX_TIME_FL_LOCAL)
            time = mktime(&t->local_date);
        else
            time = mktime(&t->gm_date);

        if (time == (time_t)-1)
            return std_system_error(ctxt, "mktime");

        t->time = (OX_Number)time * 1000 + ms;
        t->flags |= OX_TIME_FL_TIME;
    }

    *n = t->time;
    return OX_OK;
}

/*Get local date value.*/
static struct tm*
get_local_date_value (OX_Context *ctxt, OX_Time *t)
{
    if (!(t->flags & OX_TIME_FL_LOCAL)) {
        time_t time = t->time / 1000;

#ifdef ARCH_WIN
        localtime_s(&t->local_date, &time);
#else
        localtime_r(&time, &t->local_date);
#endif

        t->flags |= OX_TIME_FL_LOCAL;
    }

    return &t->local_date;
}

/*Get UTC date value.*/
static struct tm*
get_gm_date_value (OX_Context *ctxt, OX_Time *t)
{
    if (!(t->flags & OX_TIME_FL_UTC)) {
        time_t time = t->time / 1000;

#ifdef ARCH_WIN
        gmtime_s(&t->gm_date, &time);
#else
        gmtime_r(&time, &t->gm_date);
#endif

        t->flags |= OX_TIME_FL_UTC;
    }

    return &t->gm_date;
}

/*Time.$inf.$to_num*/
static OX_Result
Time_inf_to_num (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    OX_Number n;
    OX_Result r;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = get_time_value(ctxt, t, &n)) == OX_ERR)
        return r;

    ox_value_set_number(ctxt, rv, n);
    return OX_OK;
}

static const char* wday_tab[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char* mon_tab[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/*Time.$inf.$to_str*/
static OX_Result
Time_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    OX_Number time;
    int ms;
    struct tm *date;
    OX_Result r;
    char buf[64];

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = get_time_value(ctxt, t, &time)) == OX_ERR)
        return r;

    date = get_local_date_value(ctxt, t);
    ms = fmod(time, 1000);

    snprintf(buf, sizeof(buf), "%s %s %d %02d:%02d:%02d.%03d %d",
            wday_tab[date->tm_wday],
            mon_tab[date->tm_mon],
            date->tm_mday,
            date->tm_hour,
            date->tm_min,
            date->tm_sec,
            ms,
            date->tm_year + 1900);

    return ox_string_from_char_star(ctxt, rv, buf);
}

/*Time.$inf.gm_str*/
static OX_Result
Time_inf_gm_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    OX_Number time;
    int ms;
    struct tm *date;
    OX_Result r;
    char buf[64];

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = get_time_value(ctxt, t, &time)) == OX_ERR)
        return r;

    date = get_gm_date_value(ctxt, t);
    ms = fmod(time, 1000);

    snprintf(buf, sizeof(buf), "%s %s %d %02d:%02d:%02d.%03d %d",
            wday_tab[date->tm_wday],
            mon_tab[date->tm_mon],
            date->tm_mday,
            date->tm_hour,
            date->tm_min,
            date->tm_sec,
            ms,
            date->tm_year + 1900);

    return ox_string_from_char_star(ctxt, rv, buf);
}

/*Time.$inf.time get*/
static OX_Result
Time_inf_time_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    OX_Number time;
    OX_Result r;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = get_time_value(ctxt, t, &time)) == OX_ERR)
        return r;

    ox_value_set_number(ctxt, rv, time);
    return OX_OK;
}

/*Time.$inf.time set*/
static OX_Result
Time_inf_time_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    OX_Result r;
    OX_Time *t;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    if (!isfinite(n))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be a finite value"), "time");

    t->time = n;
    t->flags &= ~(OX_TIME_FL_LOCAL|OX_TIME_FL_UTC);

    return OX_OK;
}

/*Time.$inf.year get*/
static OX_Result
Time_inf_year_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_local_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_year + 1900);
    return OX_OK;
}

/*Time.$inf.year set*/
static OX_Result
Time_inf_year_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    if (!isfinite(n))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be a finite value"), "year");

    date = get_local_date_value(ctxt, t);

    date->tm_year = n - 1900;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_UTC);

    return OX_OK;
}

/*Time.$inf.mon get*/
static OX_Result
Time_inf_mon_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_local_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_mon + 1);
    return OX_OK;
}

/*Time.$inf.mon set*/
static OX_Result
Time_inf_mon_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    int mon;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    mon = n;

    if ((mon < 1) || (mon > 12))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 1 ~ 12"), "mon");

    date = get_local_date_value(ctxt, t);

    date->tm_mon = mon - 1;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_UTC);

    return OX_OK;
}

/*Time.$inf.mday get*/
static OX_Result
Time_inf_mday_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_local_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_mday);
    return OX_OK;
}

/*Time.$inf.mday set*/
static OX_Result
Time_inf_mday_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    int mday;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    mday = n;
    if ((mday < 1) || (mday > 31))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 1 ~ 31"), "mday");

    date = get_local_date_value(ctxt, t);
    date->tm_mday = mday;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_UTC);

    return OX_OK;
}

/*Time.$inf.hour get*/
static OX_Result
Time_inf_hour_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_local_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_hour);
    return OX_OK;
}

/*Time.$inf.hour set*/
static OX_Result
Time_inf_hour_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    int hour;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    hour = n;

    if ((hour < 0) || (hour > 23))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 0 ~ 23"), "hour");

    date = get_local_date_value(ctxt, t);
    date->tm_hour = hour;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_UTC);

    return OX_OK;
}

/*Time.$inf.min get*/
static OX_Result
Time_inf_min_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_local_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_min);
    return OX_OK;
}

/*Time.$inf.min set*/
static OX_Result
Time_inf_min_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    int min;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    min = n;

    if ((min < 0) || (min > 59))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 0 ~ 59"), "min");

    date = get_local_date_value(ctxt, t);
    date->tm_min = min;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_UTC);

    return OX_OK;
}

/*Time.$inf.sec get*/
static OX_Result
Time_inf_sec_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_local_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_sec);
    return OX_OK;
}

/*Time.$inf.sec set*/
static OX_Result
Time_inf_sec_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    int sec;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    sec = n;

    if ((sec < 0) || (sec > 59))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 0 ~ 59"), "sec");

    date = get_local_date_value(ctxt, t);
    date->tm_sec = sec;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_UTC);

    return OX_OK;
}

/*Time.$inf.msec get*/
static OX_Result
Time_inf_msec_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    OX_Number time;
    OX_Result r;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = get_time_value(ctxt, t, &time)) == OX_ERR)
        return r;

    ox_value_set_number(ctxt, rv, fmod(time, 1000));
    return OX_OK;
}

/*Time.$inf.msec set*/
static OX_Result
Time_inf_msec_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, time;
    int msec;
    OX_Result r;
    OX_Time *t;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    msec = n;

    if ((msec < 0) || (msec > 999))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 0 ~ 999"), "msec");

    if ((r = get_time_value(ctxt, t, &time)) == OX_ERR)
        return r;

    t->time -= fmod(t->time, 1000);
    t->time += msec;
    t->flags &= ~(OX_TIME_FL_LOCAL|OX_TIME_FL_UTC);

    return OX_OK;
}

/*Time.$inf.gm_year get*/
static OX_Result
Time_inf_gm_year_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_gm_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_year + 1900);
    return OX_OK;
}

/*Time.$inf.gm_year set*/
static OX_Result
Time_inf_gm_year_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    if (!isfinite(n))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be a finite value"), "gm_year");

    date = get_gm_date_value(ctxt, t);

    date->tm_year = n - 1900;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_LOCAL);

    return OX_OK;
}

/*Time.$inf.gm_mon get*/
static OX_Result
Time_inf_gm_mon_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_gm_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_mon);
    return OX_OK;
}

/*Time.$inf.gm_mon set*/
static OX_Result
Time_inf_gm_mon_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    int mon;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    mon = n;

    if ((mon < 0) || (mon > 11))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 0 ~ 11"), "gm_mon");

    date = get_gm_date_value(ctxt, t);

    date->tm_mon = mon;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_LOCAL);

    return OX_OK;
}

/*Time.$inf.gm_mday get*/
static OX_Result
Time_inf_gm_mday_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_gm_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_mday);
    return OX_OK;
}

/*Time.$inf.gm_mday set*/
static OX_Result
Time_inf_gm_mday_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    int mday;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    mday = n;
    if ((mday < 1) || (mday > 31))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 1 ~ 31"), "gm_mday");

    date = get_gm_date_value(ctxt, t);
    date->tm_mday = mday;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_LOCAL);

    return OX_OK;
}

/*Time.$inf.gm_hour get*/
static OX_Result
Time_inf_gm_hour_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_gm_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_hour);
    return OX_OK;
}

/*Time.$inf.gm_hour set*/
static OX_Result
Time_inf_gm_hour_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    int hour;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    hour = n;

    if ((hour < 0) || (hour > 23))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 0 ~ 23"), "gm_hour");

    date = get_gm_date_value(ctxt, t);
    date->tm_hour = hour;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_LOCAL);

    return OX_OK;
}

/*Time.$inf.gm_min get*/
static OX_Result
Time_inf_gm_min_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_gm_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_min);
    return OX_OK;
}

/*Time.$inf.gm_min set*/
static OX_Result
Time_inf_gm_min_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    int min;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    min = n;

    if ((min < 0) || (min > 59))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 0 ~ 59"), "gm_min");

    date = get_gm_date_value(ctxt, t);
    date->tm_min = min;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_LOCAL);

    return OX_OK;
}

/*Time.$inf.gm_sec get*/
static OX_Result
Time_inf_gm_sec_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    date = get_gm_date_value(ctxt, t);

    ox_value_set_number(ctxt, rv, date->tm_sec);
    return OX_OK;
}

/*Time.$inf.gm_sec set*/
static OX_Result
Time_inf_gm_sec_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    int sec;
    OX_Result r;
    OX_Time *t;
    struct tm *date;

    if (!(t = get_time(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    sec = n;

    if ((sec < 0) || (sec > 59))
        return ox_throw_range_error(ctxt, OX_TEXT("\"%s\" must be in 0 ~ 59"), "gm_sec");

    date = get_gm_date_value(ctxt, t);
    date->tm_sec = sec;
    t->flags &= ~(OX_TIME_FL_TIME|OX_TIME_FL_LOCAL);

    return OX_OK;
}


/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*?
 *? @lib Time and date functions.
 *?
 *? @class{ Time Time.
 *? The time object represents a time value in milliseconds from January 1, 1900.
 *?
 *? @func $init Initialize a time object.
 *? @param ms {?Number} The time value in milliseconds from January 1, 1900.
 *? If ms is null, initialize the time object with the current system clock time.
 *? @throw {SystemError} Cannot get the system block.
 *? @throw {RangeError} The time value cannot be an infinite value.
 *?
 *? @func $to_num Convert the time object to number.
 *? @return {Number} The time value in milliseconds from January 1, 1900.
 *?
 *? @func $to_str Convert the time object to local time string.
 *? @return {String} The string descript the local time.
 *? The format of the string similar to "Fri May 23 10:10:41.000 2025".
 *?
 *? @func gm_str Convert the time object to GMT time string.
 *? @return {String} The string descript the GMT time.
 *? The format of the string similar to "Fri May 23 10:10:41.000 2025".
 *?
 *? @acc time {Number} Time value in milliseconds from January 1, 1900.
 *? @acc year {Number} Year of the local time.
 *? @acc mon {Number} Month of the local time (1 ~ 12).
 *? @acc mday {Number} Day in the month of local time (1 ~ 31).
 *? @acc hour {Number} Hours of local time (0 ~ 23).
 *? @acc min {Number} Minutes of local time (0 ~ 59).
 *? @acc sec {Number} Seconds of local time (0 ~ 59).
 *? @acc msec {Number} Milliseconds (0 ~ 999).
 *? @acc gm_year {Number} Year of the GMT time.
 *? @acc gm_mon {Number} Month of the GMT time (1 ~ 12).
 *? @acc gm_mday {Number} Day in the month of GMT time (1 ~ 31).
 *? @acc gm_hour {Number} Hours of GMT time (0 ~ 23).
 *? @acc gm_min {Number} Minutes of GMT time (0 ~ 59).
 *? @acc gm_sec {Number} Seconds of GMT time (0 ~ 59).
 *?
 *? @class}
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, c, inf)

    /*Time.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Time"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_Time, c));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", Time_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$to_num", Time_inf_to_num));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$to_str", Time_inf_to_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "gm_str", Time_inf_gm_str));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "time", Time_inf_time_get, Time_inf_time_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "year", Time_inf_year_get, Time_inf_year_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "mon", Time_inf_mon_get, Time_inf_mon_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "mday", Time_inf_mday_get, Time_inf_mday_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "hour", Time_inf_hour_get, Time_inf_hour_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "min", Time_inf_min_get, Time_inf_min_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "sec", Time_inf_sec_get, Time_inf_sec_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "msec", Time_inf_msec_get, Time_inf_msec_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "gm_year", Time_inf_gm_year_get, Time_inf_gm_year_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "gm_mon", Time_inf_gm_mon_get, Time_inf_gm_mon_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "gm_mday", Time_inf_gm_mday_get, Time_inf_gm_mday_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "gm_hour", Time_inf_gm_hour_get, Time_inf_gm_hour_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "gm_min", Time_inf_gm_min_get, Time_inf_gm_min_set));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "gm_sec", Time_inf_gm_sec_get, Time_inf_gm_sec_set));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}
