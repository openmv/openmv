/*
 * uart_log.c -- interrupt-driven log/command port on USART1 (ST-LINK VCP).
 *
 * The HAL is used only to initialise the UART (baud rate from the real kernel clock);
 * transmit and receive run on a small register-level interrupt handler with a ring buffer,
 * so logging never blocks the frame loop unless blocking mode is selected.
 * This file is placed in ITCM by the linker script.
 */
#include <string.h>
#include "main.h"
#include "uart_log.h"

#define TX_SIZE     8192u           // power of two
#define RX_SIZE     128u

static UART_HandleTypeDef s_huart;

static char s_tx[TX_SIZE];
static volatile uint32_t s_tx_head;     // written by the main loop
static volatile uint32_t s_tx_tail;     // written by the interrupt
static volatile uint32_t s_tx_dropped;
static bool s_blocking;

static char s_rx[RX_SIZE];
static volatile uint32_t s_rx_len;
static volatile bool s_rx_ready;        // a full line is waiting in s_rx
static volatile bool s_rx_activity;

void ulog_init(uint32_t baud) {
    s_huart.Instance = LOG_UART;
    s_huart.Init.BaudRate = baud;
    s_huart.Init.WordLength = UART_WORDLENGTH_8B;
    s_huart.Init.StopBits = UART_STOPBITS_1;
    s_huart.Init.Parity = UART_PARITY_NONE;
    s_huart.Init.Mode = UART_MODE_TX_RX;
    s_huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    s_huart.Init.OverSampling = UART_OVERSAMPLING_16;
    s_huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    s_huart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    s_huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&s_huart) != HAL_OK) {
        Error_Handler();
    }
    HAL_UARTEx_DisableFifoMode(&s_huart);
    s_tx_head = s_tx_tail = 0;
    s_rx_len = 0;
    s_rx_ready = false;
    LOG_UART->ICR = USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_NECF | USART_ICR_PECF;
    LOG_UART->CR1 |= USART_CR1_RXNEIE_RXFNEIE;
    HAL_NVIC_SetPriority(LOG_UART_IRQn, IRQ_PRI_UART, 0);
    HAL_NVIC_EnableIRQ(LOG_UART_IRQn);
}

void ulog_irq_handler(void) {
    USART_TypeDef *u = LOG_UART;
    uint32_t isr = u->ISR;

    if (isr & (USART_ISR_ORE | USART_ISR_FE | USART_ISR_NE | USART_ISR_PE)) {
        u->ICR = USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_NECF | USART_ICR_PECF;
    }
    if (isr & USART_ISR_RXNE_RXFNE) {
        char c = (char) (u->RDR & 0xFF);
        if (c != '\r' && c != '\n') {
            s_rx_activity = true;       // a key press (line endings of the last command don't count)
        }
        if (!s_rx_ready) {
            if (c == '\r' || c == '\n') {
                if (s_rx_len > 0) {
                    s_rx[s_rx_len] = 0;
                    s_rx_ready = true;
                }
            } else if (c == 0x08 || c == 0x7F) {
                if (s_rx_len > 0) {
                    s_rx_len--;
                }
            } else if (s_rx_len < RX_SIZE - 1) {
                s_rx[s_rx_len++] = c;
            }
        }
    }
    if ((u->CR1 & USART_CR1_TXEIE_TXFNFIE) && (isr & USART_ISR_TXE_TXFNF)) {
        uint32_t t = s_tx_tail;
        if (t != s_tx_head) {
            u->TDR = (uint8_t) s_tx[t & (TX_SIZE - 1)];
            s_tx_tail = t + 1;
        } else {
            u->CR1 &= ~USART_CR1_TXEIE_TXFNFIE;
        }
    }
}

void ulog_write(const char *s, size_t n) {
    for (size_t i = 0; i < n; i++) {
        while ((uint32_t) (s_tx_head - s_tx_tail) >= TX_SIZE) {
            if (!s_blocking) {
                s_tx_dropped += (uint32_t) (n - i);
                goto kick;
            }
        }
        s_tx[s_tx_head & (TX_SIZE - 1)] = s[i];
        __DMB();
        s_tx_head = s_tx_head + 1;
        if ((i & 63) == 63) {
            LOG_UART->CR1 |= USART_CR1_TXEIE_TXFNFIE;   // start early for long writes
        }
    }
kick:
    LOG_UART->CR1 |= USART_CR1_TXEIE_TXFNFIE;
}

void ulog_puts(const char *s) {
    ulog_write(s, strlen(s));
}

void ulog_set_blocking(bool blocking) {
    s_blocking = blocking;
}

void ulog_flush(void) {
    while (s_tx_head != s_tx_tail) {
    }
    while (!(LOG_UART->ISR & USART_ISR_TC)) {
    }
}

uint32_t ulog_dropped(void) {
    return s_tx_dropped;
}

bool ulog_getline(char *buf, size_t size) {
    if (!s_rx_ready) {
        return false;
    }
    size_t n = s_rx_len;
    if (n >= size) {
        n = size - 1;
    }
    memcpy(buf, s_rx, n);
    buf[n] = 0;
    s_rx_len = 0;
    __DMB();
    s_rx_ready = false;
    return true;
}

bool ulog_rx_activity(void) {
    bool a = s_rx_activity;
    s_rx_activity = false;
    return a;
}

// ---- formatter ------------------------------------------------------------------------
typedef struct {
    char *buf;          // NULL: send to the UART
    size_t size, len;
    char tmp[64];       // UART staging buffer
    size_t tlen;
} out_t;

static void out_c(out_t *o, char c) {
    if (o->buf) {
        if (o->len + 1 < o->size) {
            o->buf[o->len] = c;
        }
    } else {
        o->tmp[o->tlen++] = c;
        if (o->tlen == sizeof(o->tmp)) {
            ulog_write(o->tmp, o->tlen);
            o->tlen = 0;
        }
    }
    o->len++;
}

static void out_pad(out_t *o, const char *s, int n, int width, bool left, char pad) {
    if (!left) {
        for (int i = n; i < width; i++) {
            out_c(o, pad);
        }
    }
    for (int i = 0; i < n; i++) {
        out_c(o, s[i]);
    }
    if (left) {
        for (int i = n; i < width; i++) {
            out_c(o, ' ');
        }
    }
}

// Unsigned to digits (reverse into the end of b); returns the start pointer.
static char *utoa_rev(char *end, uint32_t v, unsigned base, bool upper) {
    const char *dig = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char *p = end;
    do {
        *--p = dig[v % base];
        v /= base;
    } while (v);
    return p;
}

static int fmt_float(char *b, double v, int prec) {
    // Fixed-point formatting, enough for measurements (|v| < 4e9).
    char *p = b;
    if (v != v) {
        memcpy(b, "nan", 3);
        return 3;
    }
    if (v < 0) {
        *p++ = '-';
        v = -v;
    }
    if (prec > 6) {
        prec = 6;
    }
    uint32_t scale = 1;
    for (int i = 0; i < prec; i++) {
        scale *= 10;
    }
    if (v > 4.0e9) {
        memcpy(p, "big", 3);
        return (int) (p - b) + 3;
    }
    double r = v * scale + 0.5;
    uint32_t ip = (uint32_t) (r / scale);
    uint32_t fp = (uint32_t) (r - (double) ip * scale);
    if (fp >= scale) {          // rounding carried into the integer part
        ip++;
        fp -= scale;
    }
    char t[12];
    char *s = utoa_rev(t + sizeof(t), ip, 10, false);
    size_t n = (size_t) (t + sizeof(t) - s);
    memcpy(p, s, n);
    p += n;
    if (prec > 0) {
        *p++ = '.';
        s = utoa_rev(t + sizeof(t), fp, 10, false);
        n = (size_t) (t + sizeof(t) - s);
        for (int i = (int) n; i < prec; i++) {
            *p++ = '0';
        }
        memcpy(p, s, n);
        p += n;
    }
    return (int) (p - b);
}

static int vformat(out_t *o, const char *fmt, va_list ap) {
    for (const char *f = fmt; *f; f++) {
        if (*f != '%') {
            out_c(o, *f);
            continue;
        }
        f++;
        bool left = false;
        char pad = ' ';
        int width = 0, prec = -1;
        for (;; f++) {
            if (*f == '-') {
                left = true;
            } else if (*f == '0') {
                pad = '0';
            } else {
                break;
            }
        }
        while (*f >= '0' && *f <= '9') {
            width = width * 10 + (*f++ - '0');
        }
        if (*f == '.') {
            f++;
            prec = 0;
            while (*f >= '0' && *f <= '9') {
                prec = prec * 10 + (*f++ - '0');
            }
        }
        while (*f == 'l' || *f == 'h' || *f == 'z') {
            f++;
        }
        char t[40];
        char *s;
        int n;
        switch (*f) {
            case 'd':
            case 'i': {
                int32_t v = va_arg(ap, int32_t);
                uint32_t u = v < 0 ? (uint32_t) (-(int64_t) v) : (uint32_t) v;
                s = utoa_rev(t + sizeof(t), u, 10, false);
                if (v < 0) {
                    if (pad == '0' && width > 0) {
                        out_c(o, '-');
                        width--;
                    } else {
                        *--s = '-';
                    }
                }
                out_pad(o, s, (int) (t + sizeof(t) - s), width, left, pad);
                break;
            }
            case 'u':
            case 'x':
            case 'X': {
                uint32_t v = va_arg(ap, uint32_t);
                s = utoa_rev(t + sizeof(t), v, *f == 'u' ? 10 : 16, *f == 'X');
                out_pad(o, s, (int) (t + sizeof(t) - s), width, left, pad);
                break;
            }
            case 'c':
                t[0] = (char) va_arg(ap, int);
                out_pad(o, t, 1, width, left, ' ');
                break;
            case 's': {
                const char *str = va_arg(ap, const char *);
                if (!str) {
                    str = "(null)";
                }
                n = (int) strlen(str);
                if (prec >= 0 && n > prec) {
                    n = prec;
                }
                out_pad(o, str, n, width, left, ' ');
                break;
            }
            case 'f':
            case 'g':
            case 'e':
                n = fmt_float(t, va_arg(ap, double), prec < 0 ? 6 : prec);
                out_pad(o, t, n, width, left, pad);
                break;
            case '%':
                out_c(o, '%');
                break;
            case 0:
                f--;
                break;
            default:
                out_c(o, '%');
                out_c(o, *f);
                break;
        }
    }
    return (int) o->len;
}

int ulog_vprintf(const char *fmt, va_list ap) {
    out_t o = { 0 };
    int n = vformat(&o, fmt, ap);
    if (o.tlen) {
        ulog_write(o.tmp, o.tlen);
    }
    return n;
}

int ulog_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = ulog_vprintf(fmt, ap);
    va_end(ap);
    return n;
}

int ulog_snprintf(char *buf, size_t size, const char *fmt, ...) {
    out_t o = { 0 };
    o.buf = buf;
    o.size = size;
    va_list ap;
    va_start(ap, fmt);
    int n = vformat(&o, fmt, ap);
    va_end(ap);
    if (size) {
        buf[o.len < size ? o.len : size - 1] = 0;
    }
    return n;
}
