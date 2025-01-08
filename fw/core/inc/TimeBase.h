/**
 * @file     TimeBase.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     06.01.2025
 */

#ifndef TIMEBASE_H
#define TIMEBASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint32_t TickType_t;
extern volatile TickType_t ms_ticks;

/* Enable systick @ fHCLK with 1 ms reload period */
void timeBaseInit(void);

#ifdef __cplusplus
}
#endif

#endif  // TIMEBASE_H
