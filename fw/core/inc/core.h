/**
 * @file     core.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     06.01.2025
 */

#ifndef CORE_H
#define CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint32_t TickType_t;

/* Enable systick @ fHCLK with 1 ms reload period */
void sysTickInit(void);

/* Return current time in ms */
TickType_t sysTickTimeMs(void);

#ifdef __cplusplus
}
#endif

#endif // CORE_H
