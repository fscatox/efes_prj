/**
 * @file     debug.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     23.01.2025
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <cinttypes>
#include <cstdarg>
#include <cstdio>
#include <source_location>

/* Injects std::source_location::current() at the callee site */
#define PRINTD(fmt, ...)                                                       \
  print_impl(std::source_location::current(), stdout, fmt, ##__VA_ARGS__)
#define PRINTE(fmt, ...)                                                       \
  print_impl(std::source_location::current(), stderr, fmt, ##__VA_ARGS__)

constexpr void print_impl(const std::source_location &loc, FILE *stream,
                           const char *fmt, ...) {
#if defined(DEBUG) && (PRINT_ENABLE != 0U)
  fprintf(stream, "%s: line %" PRIuLEAST32 ":\r\n%s:\r\n\t", loc.file_name(),
          loc.line(), loc.function_name());
  va_list args;
  va_start(args, fmt);
  vfprintf(stream, fmt, args);
  fprintf(stream, "\r\n\n");
  va_end(args);
#endif // DEBUG
}

#endif // DEBUG_H
