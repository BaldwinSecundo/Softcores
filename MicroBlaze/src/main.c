#include "xil_printf.h"
#include "xil_io.h"
#include "xparameters.h"

#define GPIO_BASEADDR   XPAR_AXI_GPIO_0_BASEADDR
#define GPIO_DATA       (GPIO_BASEADDR + 0x00)   // Data register
#define GPIO_TRI        (GPIO_BASEADDR + 0x04)   // Direction (0 = output)

#define DELAY           40000000

int main()
{
    xil_printf("\r\n=== Direct GPIO Blink ===\r\n");

    // Ustaw kierunek na output
    Xil_Out32(GPIO_TRI, 0x0);

    while (1) {
        Xil_Out32(GPIO_DATA, 0x1);          // LED ON
        for (volatile int i = 0; i < DELAY; i++);
        xil_printf("\r\n=== Direct GPIO Blink ===\r\n");
        Xil_Out32(GPIO_DATA, 0x0);          // LED OFF
        for (volatile int i = 0; i < DELAY; i++);
    }

    return 0;
}
