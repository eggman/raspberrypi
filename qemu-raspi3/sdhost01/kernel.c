#include <stddef.h>
#include <stdint.h>

static inline void io_halt(void)
{
    asm volatile ("wfi");
}

#define UART0_DR ((volatile unsigned int*)(0x3F201000))
#define UART0_FR ((volatile unsigned int*)(0x3F201018))

void uart_putc(uint8_t c)
{
    // Wait for UART to become ready to transmit.
    while (*UART0_FR & (1 << 5)) { }
    *UART0_DR = c;
}
 
void uart_puts(const char *str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putc((uint8_t)str[i]);
}

void uart_hex(uint64_t num)
{
    uint64_t n = 0;
    uint64_t base = 16;
    uint64_t d = 1;
    char buf[32], *bf;

    bf = buf;

    while ( num / d >= base)
        d *= base;

    while (d != 0) {
        uint64_t dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = dgt + (dgt < 10 ? '0' : ('A') - 10);
            ++n;
        }
    }
    *bf = 0;
    
    uart_puts(buf);
}

//GPIO
#define GPFSEL4         ((volatile uint32_t*)(0x3F200010))
#define GPFSEL5         ((volatile uint32_t*)(0x3F200014))
#define GPPUD           ((volatile uint32_t*)(0x3F200094))
#define GPPUDCLK1       ((volatile uint32_t*)(0x3F20009C))

void wait_cycles(uint32_t n)
{
    if(n) while(n--) { asm volatile("nop"); }
}

void wait_msec(uint32_t num)
{
    (void) num;
}

#define SDCMD         ((volatile unsigned int*)(0x3F202000))
#define SDARG         ((volatile unsigned int*)(0x3F202004))
#define SDTOUT        ((volatile unsigned int*)(0x3F202008))
#define SDCDIV        ((volatile unsigned int*)(0x3F20200C))
#define SDRSP0        ((volatile unsigned int*)(0x3F202010))
#define SDRSP1        ((volatile unsigned int*)(0x3F202014))
#define SDRSP2        ((volatile unsigned int*)(0x3F202018))
#define SDRSP3        ((volatile unsigned int*)(0x3F20201C))
#define SDHSTS        ((volatile unsigned int*)(0x3F202020))
#define SDVDD         ((volatile unsigned int*)(0x3F202030))
#define SDEDM         ((volatile unsigned int*)(0x3F202034))
#define  SDVDD_POWER_ON  1
#define  SDVDD_POWER_OFF 0
#define SDHCFG        ((volatile unsigned int*)(0x3F202038))
#define SDHBCT        ((volatile unsigned int*)(0x3F20203C))
#define  HCFG_BUSY_IRPT_EN   0x400
#define SDDATA        ((volatile unsigned int*)(0x3F202040))
#define SDHBLC        ((volatile unsigned int*)(0x3F202050))

#define CMD_NEW_FLAG 0x8000
#define CMD_CMD_MASK 0x03FF

#define CMD0   0x0000
#define CMD2   0x0002
#define CMD8   0x0008
#define CMD55  0x0037
#define ACMD41 0x0029

uint64_t sdhost_cmd(uint16_t opcode, uint32_t arg)
{
    uint64_t r;
    uint32_t sdcmd;

    uart_puts("cmd  "); uart_hex(opcode); uart_puts(" arg "); uart_hex(arg); uart_putc('\n');

    // wait busy

    // clear error flag

    *SDARG = arg;
    sdcmd = opcode & CMD_CMD_MASK;

    // send command
    *SDCMD = sdcmd | CMD_NEW_FLAG;

    // wait resp

    r = ((uint64_t) *SDRSP1 << 32) | *SDRSP0; 
    uart_puts("resp "); uart_hex(*SDRSP3); uart_putc(' '); uart_hex(*SDRSP2); uart_putc(' '); uart_hex(*SDRSP1); uart_putc(' '); uart_hex(*SDRSP0); uart_putc('\n');

    return r;
}

void sdhost_init(void)
{
    uint32_t r;

    // GPIO_CLK, GPIO_CMD
    r=*GPFSEL4; r|=(4<<(8*3))|(4<<(9*3)); *GPFSEL4=r;
    *GPPUD=2; wait_cycles(150); *GPPUDCLK1=(1<<16)|(1<<17); wait_cycles(150); *GPPUD=0; *GPPUDCLK1=0;

    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r=*GPFSEL5; r|=(4<<(0*3)) | (4<<(1*3)) | (4<<(2*3)) | (4<<(3*3)); *GPFSEL5=r;
    *GPPUD=2; wait_cycles(150);
    *GPPUDCLK1=(1<<18) | (1<<19) | (1<<20) | (1<<21);
    wait_cycles(150); *GPPUD=0; *GPPUDCLK1=0;
 
    // init hardware
    *SDVDD = SDVDD_POWER_OFF;
    *SDCMD = 0;
    *SDARG = 0;
    *SDTOUT = 0xF00000;
    *SDCDIV = 0;
    *SDHSTS = 0x7F8;
    *SDHCFG = 0;
    *SDHBCT = 0;
    *SDHBLC = 0;

    wait_msec(20);
    *SDVDD = SDVDD_POWER_ON;
    wait_msec(20);

    *SDHCFG = HCFG_BUSY_IRPT_EN;

    sdhost_cmd(CMD0, 0);

    sdhost_cmd(CMD8, 0x000001AA);

    sdhost_cmd(CMD55, 0);

    sdhost_cmd(ACMD41, 0x51FF8000);

    sdhost_cmd(CMD2, 0);
}

void kernel_main(void)
{
    uart_puts("sdhost01\n");

    sdhost_init();

    while (1) {
        io_halt();
    }
}
