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
 * Log test.
 */

#define OX_LOG_TAG "log_test"

#include "test.h"

static void
log_output (OX_Context *ctxt)
{
    OX_LOG_D(ctxt, "debug");
    OX_LOG_I(ctxt, "information");
    OX_LOG_W(ctxt, "warning");
    OX_LOG_E(ctxt, "error");
    OX_LOG_F(ctxt, "fatal error");
}

static void
log_field_output (OX_Context *ctxt, OX_LogField field)
{
    fprintf(stderr, "set log fields: %d\n", field);
    ox_log_set_fields(ctxt, field);
    log_output(ctxt);
}

void
log_test (OX_Context *ctxt)
{
    OX_LogLevel level;
    FILE *fp;

    for (level = OX_LOG_LEVEL_ALL; level <= OX_LOG_LEVEL_NONE; level ++) {
        ox_log_set_level(ctxt, level);
        fprintf(stderr, "set log level to %d\n", level);
        log_output(ctxt);
    }

    ox_log_set_level(ctxt, OX_LOG_LEVEL_ALL);

    log_field_output(ctxt, OX_LOG_FIELD_LEVEL);
    log_field_output(ctxt, OX_LOG_FIELD_LEVEL|OX_LOG_FIELD_TAG);
    log_field_output(ctxt, OX_LOG_FIELD_LEVEL|OX_LOG_FIELD_THREAD);
    log_field_output(ctxt, OX_LOG_FIELD_LEVEL|OX_LOG_FIELD_DATE);
    log_field_output(ctxt, OX_LOG_FIELD_LEVEL|OX_LOG_FIELD_TIME);
    log_field_output(ctxt, OX_LOG_FIELD_LEVEL|OX_LOG_FIELD_MSEC);
    log_field_output(ctxt, OX_LOG_FIELD_LEVEL|OX_LOG_FIELD_FILE|OX_LOG_FIELD_LINE);
    log_field_output(ctxt, OX_LOG_FIELD_LEVEL|OX_LOG_FIELD_FUNC);
    log_field_output(ctxt, OX_LOG_FIELD_LEVEL|OX_LOG_FIELD_TIME|OX_LOG_FIELD_MSEC);

    ox_log_set_fields(ctxt, OX_LOG_FIELD_ALL);

    fp = fopen("log.txt", "ab");
    ox_log_set_file(ctxt, fp);

    log_output(ctxt);

    ox_log_set_file(ctxt, stderr);

    fclose(fp);
}
