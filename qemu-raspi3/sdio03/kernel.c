#include <stddef.h>
#include <stdint.h>

static inline void io_halt(void)
{
    asm volatile ("wfi");
}

static inline void enable_irq(void)
{
    asm volatile ("msr daifclr, #2");
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
    for (int i = 28; i >= 0; i -= 4)
        uart_putchar(hexdigits[(v >> i) & 0xf]);
}

#define SD_OK                0
#define SD_TIMEOUT          -1
#define SD_ERROR            -2

/* EMMC registers */
#define EMMC_BLKSIZECNT    ((volatile unsigned int*)(0x3F300004))
#define EMMC_ARG1          ((volatile unsigned int*)(0x3F300008))
#define EMMC_CMDTM         ((volatile unsigned int*)(0x3F30000C))
#define EMMC_RESP0         ((volatile unsigned int*)(0x3F300010))
#define EMMC_RESP1         ((volatile unsigned int*)(0x3F300014))
#define EMMC_RESP2         ((volatile unsigned int*)(0x3F300018))
#define EMMC_RESP3         ((volatile unsigned int*)(0x3F30001C))
#define EMMC_DATA          ((volatile unsigned int*)(0x3F300020))
#define EMMC_STATUS        ((volatile unsigned int*)(0x3F300024))
#define EMMC_STATUS_CMD_INHIBIT 0x00000001
#define EMMC_INTERRUPT     ((volatile unsigned int*)(0x3F300030))
#define INT_DTO_ERROR       0x00100000
#define INT_CTO_ERROR       0x00010000
#define INT_CMD_DONE        0x00000001
#define INT_ERROR_MASK      0x017E8000
#define EMMC_CONTROL0      ((volatile unsigned int*)(0x3F300028))
#define EMMC_CONTROL1      ((volatile unsigned int*)(0x3F30002C))
#define C1_SRST_DAT         0x04000000
#define C1_SRST_CMD         0x02000000
#define C1_SRST_HC          0x01000000
#define C1_TOUNIT_MAX       0x000e0000
#define C1_CLK_EN           0x00000004
#define C1_CLK_STABLE       0x00000002
#define C1_CLK_INTLEN       0x00000001
#define EMMC_IRPT_MASK      ((volatile unsigned int*)(0x3F300034))
#define EMMC_IRPT_EN        ((volatile unsigned int*)(0x3F300038))

/* command flags */
#define CMD_NEED_APP        0x80000000
#define CMD_ERRORS_MASK     0xfff9c004
#define CMD_RCA_MASK        0xffff0000

/* sd commands */
#define CMD_GO_IDLE         0x00000000
#define CMD_SEND_REL_ADDR   0x03020000
#define CMD5                0x05020000
#define CMD7                0x07020000
#define CMD_APP_CMD         0x37000000
#define CMD52               0x34020000
#define CMD53               0x35220010

static uint32_t sd_err;
static uint64_t sd_rca;
static uint64_t flg = 0;

void wait_msec(uint32_t num)
{
    register unsigned long f, t, r;
    // get the current counter frequency
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    // calculate expire value for counter
    t += ((f / 1000) * num) / 1000;
    do { asm volatile ("mrs %0, cntpct_el0" : "=r"(r)); } while (r < t);
}

/* wait for command ready */
int32_t sdhci_status(uint32_t mask)
{
    int cnt = 500000;

    while(  (*EMMC_STATUS & mask) && 
           !(*EMMC_INTERRUPT & INT_ERROR_MASK) &&
           cnt--) {
        wait_msec(1);
    }
    return (cnt <= 0 || (*EMMC_INTERRUPT & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

int32_t sdhci_set_clk(uint32_t f)
{
    (void) f;
    *EMMC_CONTROL1 &= ~C1_CLK_EN;

    // todo  setup clock divider

    *EMMC_CONTROL1 |= C1_CLK_EN;
    wait_msec(10);
 
    return SD_OK;
}

int64_t sdhci_cmd(uint32_t code, uint32_t arg)
{
	uint32_t cnt;
    uint64_t r = 0;

    sd_err = SD_OK;

    // need to send CMD_APP before ACMD* .
    if(code & CMD_NEED_APP) {
        r = sdhci_cmd(CMD_APP_CMD, 0);
        if (sd_err != SD_OK) { uart_puts("ERROR: failed to send SD APP command\n"); sd_err = SD_ERROR;return 0;}
        code &= ~CMD_NEED_APP;
    }

    // check sd host status.
    if(sdhci_status(EMMC_STATUS_CMD_INHIBIT)) { uart_puts("ERROR: EMMC busy\n"); sd_err = SD_TIMEOUT;return 0;}

    // send comand
	flg = 0;
    uart_puts("EMMC: Sending command ");
    uart_puthex(code); uart_puts(" arg "); uart_puthex(arg); uart_putchar('\n');
    *EMMC_INTERRUPT = *EMMC_INTERRUPT;  // clear intterupt
    *EMMC_ARG1 = arg;
    *EMMC_CMDTM = code;

    // wait CMD_DONE
    cnt = 10000; do{ wait_msec(10); } while( flg == 0  && cnt-- );
	flg = 0;

    // no resp
    if(code == CMD_GO_IDLE) return 0; 
    if(code == CMD_APP_CMD) return 0;

    // todo wait for resp.

    // recieve resp
    r = *EMMC_RESP0;
    uart_puts("EMMC: resp "); uart_puthex(*EMMC_RESP0); uart_putchar(' '); uart_puthex(*EMMC_RESP1); uart_putchar(' '); uart_puthex(*EMMC_RESP2); uart_putchar('\n');

    // check resp
    if(code == CMD_SEND_REL_ADDR) {
        sd_err = (( r & 0x1fff) | ((r & 0x2000) << 6) | ((r & 0x4000) << 8) | ((r & 0x8000) << 8)) & CMD_ERRORS_MASK;
        return r & CMD_RCA_MASK;
    }

    return 0;
}

#define IRQ_BASIC   ((volatile uint32_t *)(0x3F00B200))
#define IRQ_PEND2   ((volatile uint32_t *)(0x3F00B208))
#define IRQ_ENABLE2 ((volatile uint32_t *)(0x3F00B214))
#define GPU_INTERRUPTS_ROUTING ((volatile uint32_t *)(0x4000000C))
#define CORE0_INTERRUPT_SOURCE ((volatile uint32_t *)(0x40000060))

void sdhci_handler(void)
{
	uint32_t r = *EMMC_INTERRUPT;
	if (r & INT_CMD_DONE) {
        // if error occur then clear interrupt
        *EMMC_INTERRUPT = r;
    }
	uart_puts("sdhci_handler\n");
	flg = 1;
}

void c_irq_handler(void)
{
    // check inteerupt source
    if (*CORE0_INTERRUPT_SOURCE & (1 << 8)) {
        if (*IRQ_BASIC & (1 << 9)) {
            if (*IRQ_PEND2 & (1 << 30)) {
                sdhci_handler();
            }
        }
    }
    return;
}

int32_t sdhci_init(void)
{
    uint32_t cnt;

    // todo GPIO setting

    // reset host controller.
    *EMMC_CONTROL0 = 0;
    *EMMC_CONTROL1 |= C1_SRST_HC;
    cnt = 10000; do{ wait_msec(10); } while( (*EMMC_CONTROL1 & C1_SRST_HC) && cnt-- );
    if(cnt<=0) {
        uart_puts("ERROR: failed to reset EMMC\n");
        return SD_ERROR;
    }
    uart_puts("reset EMMC ok\n");

    // internal clock enable, set timeout to max ( TMCLK * 2^27 ).
    *EMMC_CONTROL1 |= C1_CLK_INTLEN | C1_TOUNIT_MAX;

    // check initial clock stable
    cnt = 10000; do{ wait_msec(10); } while( (*EMMC_CONTROL1 & C1_CLK_STABLE) && cnt-- );

    // set clock to 400KHz
    sdhci_set_clk(400000);

    // set interrupt mask.
    *EMMC_IRPT_EN   = 0x00ff0001;
    *EMMC_IRPT_MASK = 0x00ff0001;

    sd_rca = sd_err = 0;

    // send CMD0 GO_IDLE_STATE
    sdhci_cmd(CMD_GO_IDLE, 0);
    if(sd_err) return sd_err;

	// set voltage
    sdhci_cmd(CMD5,0);
 
    // get RCA
    sd_rca = sdhci_cmd(CMD_SEND_REL_ADDR,0);

    uart_puts("EMMC: CMD_SEND_REL_ADDR returned ");
    uart_puthex(sd_rca & 0xFFFFFFFF);
    uart_putchar(' ');
    uart_puthex(sd_rca>>32);
    uart_putchar('\n');

    // card select
    sdhci_cmd(CMD7, (uint32_t) sd_rca);

    return SD_OK;
}

void kernel_main(void)
{
	uint32_t i,arg;
	uint32_t r = 0;
    uart_puts("sdio01\n");

    // EMMC/SDHCI interrupt routing.
    *IRQ_ENABLE2 = 1 << 30;

    // IRQ routeing to CORE0.
    *GPU_INTERRUPTS_ROUTING = 0x00;

	enable_irq();

    sdhci_init();

	/* read test */
	sdhci_cmd(CMD52, 0);

    /* todo enable function 1 */

	*EMMC_BLKSIZECNT = 64;
    /* cmd53 byte read test*/
	arg = 0;
	arg |= 1 << 28; /* fn */
	arg |= 1 << 27; /* mode */
	arg |= 4; /* count */
	sdhci_cmd(CMD53, arg);

    r = *EMMC_DATA;
	uart_puthex(r);
	uart_putchar('\n');
	r = *EMMC_CONTROL1;

    /* cmd53 byte read test */
	arg = 0;
	arg |= 1 << 28; /* fn */
	arg |= 1 << 27; /* mode */
	arg |= 4; /* count */
	sdhci_cmd(CMD53, arg);

    r = *EMMC_DATA;
	uart_puthex(r&0xffffffff);
	uart_putchar('\n');

    /* cmd53 single block read test */
	arg = 0;
	arg |= 1 << 28; /* fn */
	arg |= 0 << 27; /* mode */
	arg |= 1; /* count */
	sdhci_cmd(CMD53, arg);

	for (i = 0; i < 16; i++) {
		uart_puthex(*EMMC_DATA);
		uart_putchar('\n');
	}

    /* cmd53 single block read test */
	arg = 0;
	arg |= 1 << 28; /* fn */
	arg |= 0 << 27; /* mode */
	arg |= 1; /* count */
	sdhci_cmd(CMD53, arg);

	for (i = 0; i < 16; i++) {
		uart_puthex(*EMMC_DATA);
		uart_putchar('\n');
	}


    while (1) {
        io_halt();
    }
}
