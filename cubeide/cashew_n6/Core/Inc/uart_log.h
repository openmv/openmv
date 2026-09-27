/*
 * uart_log.h -- interrupt-driven log/command port on the ST-LINK virtual COM port (USART1).
 */
#ifndef UART_LOG_H
#define UART_LOG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

void ulog_init(uint32_t baud);

// Queue text for sending. In non-blocking mode (default) text that does not fit in the
// 8 KB transmit ring is dropped and counted; in blocking mode the caller waits for space.
void ulog_write(const char *s, size_t n);
void ulog_puts(const char *s);
// printf subset: %d %i %u %x %X %c %s %% and %f/%.Nf, with '-', '0', width and precision.
// 'l' is accepted and ignored (32-bit). No heap, no newlib stdio.
int  ulog_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int  ulog_vprintf(const char *fmt, va_list ap);
int  ulog_snprintf(char *buf, size_t size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

void ulog_set_blocking(bool blocking);
void ulog_flush(void);                  // wait until everything queued has been sent
uint32_t ulog_dropped(void);            // characters dropped since boot

// Command input: returns true once a full line (CR or LF terminated) is available and
// copies it (without the terminator, NUL-terminated) into buf.
bool ulog_getline(char *buf, size_t size);
// True if a key was received since the last call (used to stop long loops).
bool ulog_rx_activity(void);

void ulog_irq_handler(void);

#endif
