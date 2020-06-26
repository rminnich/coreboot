/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <reset.h>
#include <device/pci_ops.h>
#include <pc80/keyboard.h>
#include <cpu/cpu.h>
#include <cpu/x86/msr.h>
#include <cpu/x86/lapic.h>
#include <cpu/amd/msr.h>
#include <cbfs.h>

static void sm(struct device *dev, uint32_t a, uint32_t base)
{
	pci_write_config32(dev, 0xb8, a);
	pci_write_config32(dev, 0xbc, base);
	printk(BIOS_ERR, "smn write %#x:%#x\r\n", a, base);
}

static void romepsp_nb_init(struct device *dev)
{
	print_func_entry();

	printk(BIOS_ERR, "==============================================> \n");
	printk(BIOS_ERR, "HI HTEREER\n");
		// wire down the other apics
	sm(dev, 0x02800000, 0xc9280001);
	sm(dev, 0x02900000, 0xf4180001);
	sm(dev, 0x02a00000, 0xc8180001);
	sm(dev, 0x02b00000, 0xf5180001);
	printk(BIOS_ERR, "Wrote those fuckers, check them out.\n");
		db();

	print_func_exit();
}

static void romepsp_nb_read_resources(struct device *dev)
{
	print_func_entry();
	pci_dev_read_resources(dev);

	printk(BIOS_ERR, "==============================================> \n");
	/* reserve mmconfig */
	fixed_mem_resource(dev, 2, CONFIG_MMCONF_BASE_ADDRESS >> 10, 0x10000000 >> 10,
			   IORESOURCE_RESERVE);

	print_func_exit();
}

static void nb_pci_dev_set_resources(struct device *dev)
{
	print_func_entry();
	printk(BIOS_ERR, "==============================================> \n");
	pci_dev_set_resources(dev);
	print_func_exit();
}

static void nb_pci_dev_enable_resources(struct device *dev)
{
	print_func_entry();
	printk(BIOS_ERR, "==============================================> \n");
	pci_dev_set_resources(dev);
	print_func_exit();
}

static struct device_operations nb_operations = {
	.read_resources   = romepsp_nb_read_resources,
	.set_resources    = nb_pci_dev_set_resources,
	.enable_resources = nb_pci_dev_enable_resources,
	.init             = romepsp_nb_init,
};

static const struct pci_driver nb_driver __pci_driver = {
	.ops = &nb_operations,
	.vendor = 0x1022,
	.device = 0x1480,
};

/* these are in mtrr.h and I'm not ready for that. */
#define TOP_MEM		0xC001001Aul
#define TOP_MEM2	0xC001001Dul

#define TOP_MEM_MASK			0x007fffff
#define TOP_MEM_MASK_KB			(TOP_MEM_MASK >> 10)


static void c00c01(uint8_t ix, uint8_t dd)
{
#define x (uint16_t)0xc00
#define d (uint16_t)0xc01
	printk(BIOS_SPEW, "Write to %#x val %#x: Start: Index is %#x, data %#x;", ix, dd, inb(x), inb(d));
	outb(ix, x);
	printk(BIOS_SPEW, "Prev: Index is %#x, data %#x;", inb(x), inb(d));
	outb(dd, d);
	printk(BIOS_SPEW, "Done: Index is %#x, data %#x\n", inb(x), inb(d));
#undef x
#undef d
}
static void mainboard_amd_romepsp_enable(struct device *dev)
{
	msr_t msr;
	unsigned long tomk = 0; //, high = 0;


	print_func_entry();
	printk(BIOS_INFO, "Mainboard " CONFIG_MAINBOARD_PART_NUMBER " Enable.\n");

	msr = rdmsr(MMIO_CONF_BASE);
	printk(BIOS_ERR, "c0010058 val %x:%x\n", msr.hi, msr.lo);
	printk(BIOS_ERR, "Setting c0010058 to %x\n", CONFIG_MMCONF_BASE_ADDRESS);
	msr.lo = CONFIG_MMCONF_BASE_ADDRESS | 0x21;
	wrmsr(MMIO_CONF_BASE, msr);
	msr = rdmsr(MMIO_CONF_BASE);
	printk(BIOS_ERR, "c0010058 val %x:%x\n", msr.hi, msr.lo);

	msr = rdmsr(TOP_MEM);
	printk(BIOS_ERR, "TOP %x:%x\n", msr.hi,msr.lo);
	tomk = (msr.lo>>10)&TOP_MEM_MASK;
	printk(BIOS_DEBUG, "QEMU: cmos: %lu MiB RAM below 4G.\n", tomk / 1024);
	/* Report the memory regions. */
	ram_resource(dev, 10, 0, 640);
	ram_resource(dev, 11, 1024, /*tomk - 1024);*/1024*1024);
//	if (high)
//		ram_resource(dev, idx++, 4 * 1024 * 1024, high);
	if (0)
		db();
	// Need to do this to enable ioapic:
	// >sw 13b102f0 fec00001
	// hack.
	//     wmem fed80300 e3070b77
	//    wmem fed00010 3
	uint32_t *v = (void *)0xfed80300;
	*v = 0xe3070b77;
	v = (void *)0xfed00010;
	*v = 3;
	v = (void *)0xfed00100;
	*v |= 8;
	// THis is likely not needed but.
	v = (void *)0xfed00108;
	*v = 0x5b03d997;
	do_lapic_init();

#if 0
	/* rminnich@rminnich-MacBookPro:~/AMD64/coreboot$ cpu r io rl  0xfed00108 */
	/* 	2011/01/20 00:29:52 Mounted /tmp/cpu/lib on /lib */
	/* 	2011/01/20 00:29:52 Mounted /tmp/cpu/lib64 on /lib64 */
	/* 	2011/01/20 00:29:52 Warning: mounting /tmp/cpu/lib32 on /lib32 failed: no such file or directory */
	/* 	2011/01/20 00:29:52 Mounted /tmp/cpu/usr on /usr */
	/* 	2011/01/20 00:29:52 Mounted /tmp/cpu/bin on /bin */
	/* 	2011/01/20 00:29:52 Mounted /tmp/cpu/etc on /etc */
	/* 	2011/01/20 00:29:52 Mounted /tmp/cpu/home on /home */
	/* 	0x5b03d997 */
	/* 	k */
	// 	~/> mknod /dev/mem c 1 1
	// ~/> io inb c00
	// 2011/01/24 17:02:13 strconv.ParseUint: parsing "c00": invalid syntax
	// Exception: io exited with 1
	// [tty], line 1: io inb c00
	// ~/> io inb 0xc00
	// 0x07
	// ~/> io inb 0xc01
	// 0x1f # default
	// ~/> io outb 0xc00 0
	// ~/> io inb 0xc01
	// 0x1f
	// ~/> io outb 0xc00 0x75
	// ~/> io inb 0xc01
	// 0x04
	// ~/> io outb 0xc00 0
	// ~/> io inb 0xc01
	// 0x1f
	// ~/> io outb 0xc00 8
	// ~/> io inb 0xc01
	// 0xfa
	// ~/> io out
	// io (r{b,w,l,q} address)...
	// io (w{b,w,l,q} address value)...
	// io (cr index)... # read from CMOS register index [14-127]
	// io (cw index value)... # write value to CMOS register index [14-127]
	// io (rtcr index)... # read from RTC register index [0-13]
	// io (rtcw index value)... # write value to RTC register index [0-13]
	// io (in{b,w,l} address)...
	// io (out{b,w,l} address value)...
	// Exception: io exited with 1
	// [tty], line 1: io out
	// ~/> io outb 0xc00 9
	// ~/> io inb 0xc01
	// 0x91
	// ~/> io outb 0xc00 0xa
	// ~/> io inb 0xc01
	// 0x00
	// ~/> io outb 0xc00 0xb
	// ~/> io inb 0xc01
	// 0x00
	// ~/> io outb 0xc00 0xc
	// ~/> io inb 0xc01
	// 0x1f
	// ~/>
#endif
	c00c01(0x75, 4); // UART IRQ
	c00c01(8, 0xfa); // ??
	c00c01(9, 0x91);
	print_func_exit();
}

struct chip_operations mainboard_amd_romepsp_ops = {
	.enable_dev = mainboard_amd_romepsp_enable,
};

static void mainboard_enable(struct device *dev)
{
	print_func_entry();
	mainboard_amd_romepsp_enable(dev);
	printk(BIOS_INFO, "Mainboard " CONFIG_MAINBOARD_PART_NUMBER " Enable.\n");
	print_func_exit();
}


struct chip_operations mainboard_ops = {
	.enable_dev = mainboard_enable,
};

void do_board_reset(void)
{
	print_func_entry();
	die("reset");
	print_func_exit();
}

// quite the hack.
unsigned long blob(unsigned long start)
{
	unsigned long current = start;
	acpi_header_t *f;
	size_t s;

	if (current > 0x00000000a85eb000) {
		printk(BIOS_ERR, "current is %#lx table won't fit at right place :-(\n", current);
		return 0;
	}

	printk(BIOS_ERR, "fw_cfg_acpi_tables: [    0.000000] BIOS-e820: [mem 0x00000000a85eb000-0x00000000a86c6fff] ACPI data\n");
	printk(BIOS_ERR, "Current is %#lx\n", current);

	f = cbfs_boot_map_with_leak(CONFIG_CBFS_PREFIX "/ACPIBLOB", CBFS_TYPE_RAW, &s);
	printk(BIOS_ERR, "back here is file  %p size %#lx\n", f, s);
	if (!f) {
		printk(BIOS_ERR, "No ACPI blob\n");
		return current;
	}

	memcpy((void *)0x00000000a85eb000, f, s);
	current = 0x00000000a86c6fff;
	return current;
}
