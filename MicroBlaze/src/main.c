/*
 * Przykładowa aplikacja bare-metal dla MicroBlaze V (RISC-V) na PYNQ-Z1.
 *  - miga LED podpięty do AXI GPIO,
 *  - wysyła tekst przez AXI UART Lite (9600 baud, ustawione w Vivado).
 *
 * Dostęp bezpośrednio do rejestrów - adresy zgodne z Address Editor:
 *   AXI GPIO     0x4000_0000
 *   AXI UARTLite 0x4060_0000
 */
#include <stdint.h>

#define GPIO_BASE    0x40000000u
#define GPIO_DATA    (*(volatile uint32_t *)(GPIO_BASE + 0x00))
#define GPIO_TRI     (*(volatile uint32_t *)(GPIO_BASE + 0x04))

#define UART_BASE    0x40600000u
#define UART_TX      (*(volatile uint32_t *)(UART_BASE + 0x04))
#define UART_STAT    (*(volatile uint32_t *)(UART_BASE + 0x08))
#define UART_TX_FULL (1u << 3)

static void uart_putc(char c)
{
    while (UART_STAT & UART_TX_FULL) { }
    UART_TX = (uint32_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

static void delay(volatile uint32_t n)
{
    while (n--) { }
}

int main(void)
{
    GPIO_TRI = 0x0;                 /* wszystkie piny jako wyjścia */
    uart_puts("=== Direct GPIO Blink ===\n");

    while (1) {
        GPIO_DATA = 1;
        delay(5000000);
        GPIO_DATA = 0;
        delay(5000000);
    }
    return 0;
}
