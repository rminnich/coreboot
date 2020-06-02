/* SPDX-License-Identifier: GPL-2.0-only */

#define	ASPEED_CONFIG_INDEX	0x4e
#define	ASPEED_CONFIG_DATA	0x4f

#include <device/pci_ops.h>
#include <bootblock_common.h>
#include <console/console.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/msr.h>
#include <console/console.h>
#include <drivers/uart/uart8250reg.h>
#include <superio/aspeed/ast2400/ast2400.h>
#include <superio/aspeed/common/aspeed.h>

static uint8_t com_to_ast_sio(uint8_t com)
{
	switch (com) {
	case 0:
		return AST2400_SUART1;
	case 1:
		return AST2400_SUART2;
	case 2:
		return AST2400_SUART3;
	case 4:
		return AST2400_SUART4;
	default:
		return AST2400_SUART1;
	}
}

#if 0
static void spin(u32 i)
{
	while (i-- != 0) {
		__asm__ __volatile__ ("");
	}
}

static void spew_first(u8 tag, u8 v)
{
	spin(0x100000);
	post_code(tag);
	spin(0x40000);
	if (v == 0x00)
		post_code(0x80);
	else
		post_code(0x00);
	spin(0x40000);

	post_code(v);
}

static void spew_u8(u8 idx, u8 v)
{
	u8 ii;

	idx &= 0xf;

	ii = (idx << 4) | idx;
	if (ii == v)
		ii = 0x80 | idx;

	spin(0x100000);
	post_code(ii);
	spin(0x40000);
	post_code(v);
}

static void spew32(u8 tag, u32 v)
{
	u8 v8;

	v8 = (v & 0xff000000) >> 24;
	spew_first(tag, v8);

	v8 = (v & 0xff0000) >> 16;
	spew_u8(1, v8);

	v8 = (v & 0xff00) >> 8;
	spew_u8(2, v8);

	v8 = v & 0xff;
	spew_u8(3, v8);
}
#endif

static void bootblock_northbridge_init(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	print_func_exit();
}

static void enable_spi_prefetch(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	print_func_exit();
}

static void bootblock_southbridge_init(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	enable_spi_prefetch();
	print_func_exit();
}

void bootblock_soc_init(void)
{
	print_func_entry();
	printk(BIOS_SPEW, "%s\n", __func__);
	bootblock_northbridge_init();
	bootblock_southbridge_init();
	print_func_exit();
}

#define SINGLE_CHAR_TIMEOUT	(50 * 1000)
#define FIFO_TIMEOUT		(16 * SINGLE_CHAR_TIMEOUT)

/* Enable IO access to port, then enable UART HW control pins */
static void enable_serial(unsigned int serial_base, unsigned int io_enable)
{
	u8 temp8;

	temp8 = read32((void *)0xfed803ec);
	temp8 |= 1;
	write32((void *)0xfed803ec, temp8);

	pci_write_config32(PCI_DEV(0, 0x14, 3), 0x44, 0xc0);
	pci_write_config32(PCI_DEV(0, 0x14, 3), 0x48, 0x03);
}

void bootblock_mainboard_early_init(void)
{
	u32 temp;
	msr_t msr;
	u64 mmio_base;
	/* Configure appropriate physical port of SuperIO chip off BMC */
	const pnp_devfn_t serial_dev =
	    PNP_DEV(ASPEED_CONFIG_INDEX,
	    com_to_ast_sio(CONFIG_UART_FOR_CONSOLE));
	const pnp_devfn_t gpio_dev = PNP_DEV(ASPEED_CONFIG_INDEX, AST2400_GPIO);

	post_code(0x1d);

	mmio_base = (u64)CONFIG_MMCONF_BASE_ADDRESS;
	msr = rdmsr(MMIO_CONF_BASE);
	msr.hi = mmio_base >> 32;
	msr.lo = (mmio_base & 0xfff00000ull) | 0x21;
	wrmsr(MMIO_CONF_BASE, msr);

	post_code(0x1e);

	/*sb_clk_output_48Mhz(2);*/
	temp = read32((void *)0xfed80e40ul);
	temp |= 4;
	write32((void *)0xfed80e40ul, temp);

	post_code(0x1f);

	aspeed_enable_serial(serial_dev, CONFIG_TTYS0_BASE);

	/* Port 80h direct to GPIO for LED display */
	aspeed_enable_port80_direct_gpio(gpio_dev, GPIOH);

	/* Enable UART function pin */
	aspeed_enable_uart_pin(serial_dev);

	enable_serial(0x03f8, 0x40);
}
