/*
 * dma01
 *
 * BCM2835 ARM Peripherals
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf
 * 4 DMA Controller
 *
 * BCM2836 QA7 ARM Quad A7 core
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf
 */

#include <stddef.h>
#include <stdint.h>

static inline void io_halt(void)
{
    asm volatile ("wfi");
}

#define AUX_MU_IO   ((volatile uint32_t *)(0x3F215040))
#define AUX_MU_LSR  ((volatile uint32_t *)(0x3F215054))

void uart_putchar(uint8_t c)
{
    /* wait mini uart for tx idle. */
    while ( !(*AUX_MU_LSR & (1 << 5)) ) { }
    *AUX_MU_IO = c;
}

void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putchar((uint8_t)str[i]);
}

void uart_puthex(uint64_t v)
{
    const char *hexdigits = "0123456789ABSDEF";
    for (int i = 60; i >= 0; i -= 4)
        uart_putchar(hexdigits[(v >> i) & 0xf]);
}

typedef struct{
    uint32_t transfer_information;
    uint32_t source_addr;
    uint32_t dest_addr;
    uint32_t transfer_length;
    uint32_t stride;
    uint32_t next_control_block;
    uint32_t dummy0;
    uint32_t dummy1;
} DMA_CONTROLBLOCK __attribute__ ((aligned(32)));

#define DMA_CH0_CTRL   ((volatile uint32_t *)(0x3F007000))
#define DMA_CH0_ADDR   ((volatile uint32_t *)(0x3F007004))
#define DMA_ENABLE     ((volatile uint32_t *)(0x3F007FF0))
#define DMA_INT_STATUS ((volatile uint32_t *)(0x3F007FE0))
static uint8_t src[256] = "hello world\n";
static uint8_t dst[256] = { 0 };
DMA_CONTROLBLOCK dma_control_block;

#define IRQ_BASIC   ((volatile uint32_t *)(0x3F00B200))
#define IRQ_PEND1   ((volatile uint32_t *)(0x3F00B204))
#define IRQ_ENABLE1 ((volatile uint32_t *)(0x3F00B210))
#define GPU_INTERRUPTS_ROUTING ((volatile uint32_t *)(0x4000000C))
#define CORE0_INTERRUPT_SOURCE ((volatile uint32_t *)(0x40000060))

static inline void enable_irq(void)
{
    asm volatile ("msr daifclr, #2");
}

void c_irq_handler(void)
{
    /* check inteerupt source */
    if (*CORE0_INTERRUPT_SOURCE & (1 << 8)) {
        if (*IRQ_BASIC & (1 << 8)) {
            if (*IRQ_PEND1 & (1 << 16)) {
                if (*DMA_CH0_CTRL & (1 << 2)) {
                    *DMA_CH0_CTRL |= 1 << 2;    /* clear INT */
                    uart_puts(" c_irq_handler\n");
                    uart_puts((const char *) dst);
                    return;
                }
            }
        }
    }
    return;
}

void kernel_main(void)
{
    uart_puts("dma01\n");

    *DMA_INT_STATUS = 0;
    *DMA_ENABLE |= 1 << 0; /* enable DMA ch 0 */

    *DMA_CH0_CTRL |= 1 << 31; /* set reset */

    /* setup dma control block */
    dma_control_block.transfer_information = 0;
    dma_control_block.transfer_information |= 1 << 26 | 1<< 8 | 1 << 4; /* no wide bursts, src increment, dest increment */
    dma_control_block.source_addr = (intptr_t) src;
    dma_control_block.dest_addr = (intptr_t) dst;
    dma_control_block.next_control_block = 0;
    dma_control_block.stride = 0;
    dma_control_block.transfer_length = 256; /* Multiples of 4 */
    dma_control_block.dummy0 = 0;
    dma_control_block.dummy1 = 0;

    /* interrupt */
    *GPU_INTERRUPTS_ROUTING = 0x00; /* core0 */
    *IRQ_ENABLE1 = 1 << 16; /* DMA CH0 */
    enable_irq();
    dma_control_block.transfer_information |= 1 << 0; /* INTEN */

    /* kick dma */
    *DMA_CH0_ADDR = (intptr_t) &dma_control_block;
    *DMA_CH0_CTRL |= 1 << 0; /* Active */

    while (1) {
        io_halt();
    }
}

