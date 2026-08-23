#include "can_trace.h"
#include <string.h>

static TraceRecord_t s_log[TRACE_LOG_DEPTH];
static volatile uint16_t s_head;
static volatile uint16_t s_tail;
static volatile uint32_t s_overflow_count;

void TraceLog_Init(void)
{
    memset(s_log, 0, sizeof(s_log));
    s_head = 0;
    s_tail = 0;
    s_overflow_count = 0;
}

void TraceLog_Push(const TraceRecord_t *rec)
{
    uint16_t next_head = (uint16_t)((s_head + 1) & (TRACE_LOG_DEPTH - 1));

    if (next_head == s_tail) {
        /* drop oldest to make room — bench visibility priority, see header */
        s_tail = (uint16_t)((s_tail + 1) & (TRACE_LOG_DEPTH - 1));
        s_overflow_count++;
    }
    s_log[s_head] = *rec;
    s_head = next_head;
}

bool TraceLog_Pop(TraceRecord_t *out_rec)
{
    if (s_tail == s_head) return false;

    *out_rec = s_log[s_tail];
    s_tail = (uint16_t)((s_tail + 1) & (TRACE_LOG_DEPTH - 1));
    return true;
}

uint16_t TraceLog_Depth(void)
{
    return (uint16_t)((s_head - s_tail) & (TRACE_LOG_DEPTH - 1));
}

uint32_t TraceLog_OverflowCount(void)
{
    return s_overflow_count;
}
