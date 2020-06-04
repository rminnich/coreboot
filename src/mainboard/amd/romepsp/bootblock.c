/* SPDX-License-Identifier: GPL-2.0-only */

#define	ASPEED_CONFIG_INDEX	0x4e
#define	ASPEED_CONFIG_DATA	0x4f

#include <device/pci_ops.h>
#include <bootblock_common.h>
#include <console/console.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/msr.h>
#include <console/console.h>
#include <console/uart.h>
#include <drivers/uart/uart8250reg.h>
#include <superio/aspeed/ast2400/ast2400.h>
#include <superio/aspeed/common/aspeed.h>

#include <commonlib/helpers.h>
#include <device/mmio.h>
/*
#include <amdblocks/gpio_banks.h>
#include <amdblocks/acpimmio.h>
#include <soc/southbridge.h>
#include <soc/gpio.h>
*/
#if 0
static const struct _uart_info {
	uintptr_t base;
	struct soc_amd_gpio mux[2];
} uart_info[] = {
	[0] = { APU_UART0_BASE, {
			PAD_NF(GPIO_138, UART0_TXD, PULL_NONE),
			PAD_NF(GPIO_136, UART0_RXD, PULL_NONE),
	} },
	[1] = { APU_UART1_BASE, {
			PAD_NF(GPIO_143, UART1_TXD, PULL_NONE),
			PAD_NF(GPIO_141, UART1_RXD, PULL_NONE),
	} },
	/*
	[2] = { APU_UART2_BASE, {
			PAD_NF(GPIO_137, UART2_TXD, PULL_NONE),
			PAD_NF(GPIO_135, UART2_RXD, PULL_NONE),
	} },
	[3] = { APU_UART3_BASE, {
			PAD_NF(GPIO_140, UART3_TXD, PULL_NONE),
			PAD_NF(GPIO_142, UART3_RXD, PULL_NONE),
	} },
	*/
};

uintptr_t uart_platform_base(int idx)
{
	if (idx < 0 || idx > ARRAY_SIZE(uart_info))
		return 0;

	return uart_info[idx].base;
}

void set_uart_config(int idx)
{
	uint32_t uart_ctrl;
	uint16_t uart_leg;

	if (idx < 0 || idx > ARRAY_SIZE(uart_info))
		return;

	program_gpios(uart_info[idx].mux, 2);

	if (CONFIG(PICASSO_UART_1_8MZ)) {
		uart_ctrl = sm_pci_read32(SMB_UART_CONFIG);
		uart_ctrl |= 1 << (SMB_UART_1_8M_SHIFT + idx);
		sm_pci_write32(SMB_UART_CONFIG, uart_ctrl);
	}

	if (CONFIG(PICASSO_UART_LEGACY) && idx != 3) {
		/* Force 3F8 if idx=0, 2F8 if idx=1, 3E8 if idx=2 */

		/* TODO: make clearer once PPR is updated */
		uart_leg = (idx << 8) | (idx << 10) | (idx << 12) | (idx << 14);
		if (idx == 0)
			uart_leg |= 1 << FCH_LEGACY_3F8_SH;
		else if (idx == 1)
			uart_leg |= 1 << FCH_LEGACY_2F8_SH;
		else if (idx == 2)
			uart_leg |= 1 << FCH_LEGACY_3E8_SH;

		write16((void *)FCH_UART_LEGACY_DECODE, uart_leg);
	}
}

unsigned int uart_platform_refclk(void)
{
	return CONFIG(PICASSO_UART_48MZ) ? 48000000 : 115200 * 16;
}

#endif
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

#if 0
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
#endif

void bootblock_mainboard_early_init(void)
{
	post_code(0x1d);

	post_code(0x1e);

	post_code(0x1f);
}
