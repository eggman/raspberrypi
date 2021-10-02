/*
 * sdhost02.c :  read 1st sector from sdcard
 */
#include <stddef.h>
#include <stdint.h>

static inline void io_halt(void)
{
    asm volatile ("wfi");
}

#define UART0_DR ((volatile uint32_t *)(0x3F201000))
#define UART0_FR ((volatile uint32_t *)(0x3F201018))

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

void uart_hex(uint64_t n)
{
    const char *hexdigits = "0123456789ABCDEF";
    for (int i = 60; i >= 0; i -= 4)
        uart_putc(hexdigits[(n >> i) & 0xf]);
}

void uart_dump(void *ptr)
{
    uint64_t a,b,d;
    uint8_t c;
    for ( a =(uint64_t) ptr; a < (uint64_t) ptr + 512; a += 16) {
        uart_hex(a); uart_puts(": ");
        for(b = 0; b < 16; b++) {
            c = *((uint8_t *)(a + b));
            d = (uint64_t) c; d >>= 4; d &= 0xF; d += d > 9 ? 0x37: 0x30; uart_putc(d);
            d = (uint64_t) c; d &= 0xF; d += d > 9 ? 0x37 : 0x30; uart_putc(d);
            uart_putc(' ');
            if(b % 4 == 3)
                uart_putc(' ');
        }
        for(b = 0; b < 16; b++) {
            c = *((uint8_t *)(a + b));
            uart_putc(c < 32 || c >= 127 ? '.' : c);
        }
        uart_putc('\n');
    }
}

// GPIO
#define GPFSEL4         ((volatile uint32_t*)(0x3F200010))
#define GPFSEL5         ((volatile uint32_t*)(0x3F200014))
#define GPPUD           ((volatile uint32_t*)(0x3F200094))
#define GPPUDCLK1       ((volatile uint32_t*)(0x3F20009C))

// wait
void wait_cycles(uint32_t n)
{
    if(n) while(n--) { asm volatile("nop"); }
}

void wait_msec(uint32_t num)
{
    (void) num;
}

// SD
#define SDCMD         ((volatile uint32_t *)(0x3F202000))
#define  SDCMD_NEW      (1 << 15)
#define  SDCMD_FAIL     (1 << 14)
#define  SDCMD_BUSYWAIT (1 << 11)
#define  SDCMD_NORESP   (1 << 10)
#define  SDCMD_LONGRESP (1 << 9)
#define  SDCMD_WRITE    (1 << 7)
#define  SDCMD_READ     (1 << 6)
#define  SDCMD_CMD_MASK 0x03FF
#define SDARG         ((volatile uint32_t *)(0x3F202004))
#define SDTOUT        ((volatile uint32_t *)(0x3F202008))
#define SDCDIV        ((volatile uint32_t *)(0x3F20200C))
#define SDRSP0        ((volatile uint32_t *)(0x3F202010))
#define SDRSP1        ((volatile uint32_t *)(0x3F202014))
#define SDRSP2        ((volatile uint32_t *)(0x3F202018))
#define SDRSP3        ((volatile uint32_t *)(0x3F20201C))
#define SDHSTS        ((volatile uint32_t *)(0x3F202020))
#define SDVDD         ((volatile uint32_t *)(0x3F202030))
#define  SDVDD_POWER_ON  1
#define  SDVDD_POWER_OFF 0
#define SDEDM         ((volatile uint32_t *)(0x3F202034))
#define SDHCFG        ((volatile uint32_t *)(0x3F202038))
#define  HCFG_BUSY_IRPT_EN  (1 << 10)
#define SDHBCT        ((volatile uint32_t *)(0x3F20203C))
#define SDDATA        ((volatile uint32_t *)(0x3F202040))
#define SDHBLC        ((volatile uint32_t *)(0x3F202050))

#define CMD0   0x0000
#define CMD2   0x0002
#define CMD3   0x0003
#define CMD7   0x0007
#define CMD8   0x0008
#define CMD17  0x0011
#define CMD55  0x0037
#define ACMD41 0x0029
#define ACMD51 0x0033

#define CMD3_RESP_RCA_MASK 0XFFFF0000

#define SD_OK      0
#define SD_ERROR  -2

static int32_t sd_err, sd_scr[2];
static uint64_t sd_rca;

uint64_t sdhost_cmd(uint16_t opcode, uint32_t arg)
{
    uint64_t r;
    uint32_t sdcmd;

    sd_err = SD_OK;

    // todo wait busy

    // clear error flag
    *SDARG = arg;

    // set opcode
    sdcmd = opcode & SDCMD_CMD_MASK;

    // set flag
    if (opcode == CMD0 ) { sdcmd |= SDCMD_NORESP; } else
    if (opcode == CMD2 ) { sdcmd |= SDCMD_LONGRESP; } else
    if (opcode == CMD7 ) { sdcmd |= SDCMD_BUSYWAIT; } else
    if (opcode == CMD17) { sdcmd |= SDCMD_READ; }
    if (opcode == ACMD51) { sdcmd |= SDCMD_READ; }

    // set byte count and block count
    if (opcode == CMD17 ) { *SDHBCT = 512; *SDHBLC = 1;}
    if (opcode == ACMD51) { *SDHBCT = 8; *SDHBLC = 1;}

    // send command
    sdcmd |= SDCMD_NEW;
    uart_puts("flg  "); uart_hex(sdcmd & 0xFFC0);
    uart_puts(" cmd "); uart_hex(sdcmd & 0x3F);
    uart_puts(" arg "); uart_hex(arg); uart_putc('\n');
    *SDCMD = sdcmd;

    // no resp
    if (sdcmd & SDCMD_NORESP) {return 0;}

    // todo wait resp

    // resp
    r =  *SDRSP0;
    uart_puts("resp "); uart_hex(*SDRSP3); uart_putc(' '); uart_hex(*SDRSP2); uart_putc(' '); uart_hex(*SDRSP1); uart_putc(' '); uart_hex(*SDRSP0); uart_putc('\n');

    // status check
    if (*SDCMD & SDCMD_FAIL) { uart_puts("sdcmd fail\n"); sd_err = SD_ERROR; return 0;}

    if (!(sdcmd & SDCMD_LONGRESP)) {
        if (opcode == CMD3   ||
            opcode == CMD7   ||
            opcode == CMD8   ||
            opcode == CMD17  ||
            opcode == CMD55  ||
            opcode == ACMD51 ) {
                uart_puts("state "); uart_hex((r & 0x1E00) >> 9);
                if (r & 0x0020) { uart_puts(" APP_CMD "); }
                uart_putc('\n');
        }
    }

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
    *SDHSTS = 0x7F9;
    *SDHCFG = 0;
    *SDHBCT = 0;
    *SDHBLC = 0;

    // init fifo
    r = *SDEDM;
    r &= 0x1f << 9 | 0x1f << 4;
    r |= 4 << 9 | 4 << 4;
    *SDEDM = r;

    wait_msec(20);
    *SDVDD = SDVDD_POWER_ON;
    wait_msec(20);

    *SDHCFG = HCFG_BUSY_IRPT_EN;

    sdhost_cmd(CMD0, 0);

    sdhost_cmd(CMD8, 0x000001AA);

    sdhost_cmd(CMD55, 0);

    sdhost_cmd(ACMD41, 0x51FF8000);

    sdhost_cmd(CMD2, 0);

    sd_rca = CMD3_RESP_RCA_MASK & sdhost_cmd(CMD3, 0);
}

void sdhost_cardselect(void)
{
    // card select
    sdhost_cmd(CMD7, (uint32_t) sd_rca);

    // read SCR
    sdhost_cmd(CMD55, (uint32_t) sd_rca);
    sdhost_cmd(ACMD51, 0);

    // todo check read status
    sd_scr[0] = *SDDATA;
    sd_scr[1] = *SDDATA;
    uart_puts("SCR ");
    uart_hex(sd_scr[0]); uart_putc(' ');
    uart_hex(sd_scr[1]); uart_putc(' ');
    uart_putc('\n');
}

void sdhost_readblock(void)
{
    uint32_t buf[128];
    // read single block
    sdhost_cmd(CMD17, 0);

    // todo check read status
    for (size_t i = 0; i < 128; i++) {
        buf[i] = *SDDATA;
    }
    uart_dump(buf);
}

void kernel_main(void)
{
    uart_puts("qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c\n");
    uart_puts("sdhost02\n");

    sdhost_init();
    sdhost_cardselect();
    sdhost_readblock();

    while (1) {
        io_halt();
    }
}

