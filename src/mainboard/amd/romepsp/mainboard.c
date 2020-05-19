/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ops.h>
#include <pc80/keyboard.h>
#include <cpu/cpu.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/msr.h>

static void qemu_nb_init(struct device *dev)
{
	print_func_entry();

	print_func_exit();
}

static void qemu_nb_read_resources(struct device *dev)
{
	print_func_entry();
	pci_dev_read_resources(dev);

	/* reserve mmconfig */
	fixed_mem_resource(dev, 2, CONFIG_MMCONF_BASE_ADDRESS >> 10, 0x10000000 >> 10,
			   IORESOURCE_RESERVE);

	print_func_exit();
}


static struct device_operations nb_operations = {
	.read_resources   = qemu_nb_read_resources,
	.set_resources    = pci_dev_set_resources,
	.enable_resources = pci_dev_enable_resources,
	.init             = qemu_nb_init,
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


static void mainboard_amd_romepsp_enable(struct device *dev)
{
	msr_t msr;
	unsigned long tomk = 0; //, high = 0;


	print_func_entry();
	printk(BIOS_INFO, "Mainboard " CONFIG_MAINBOARD_PART_NUMBER " Enable.\n");

	msr = rdmsr(MMIO_CONF_BASE);
	printk(BIOS_ERR, "c0010058 val %x:%x\n", msr.hi, msr.lo);
	printk(BIOS_ERR, "Setting c0010058 to %x\n", msr.lo | 0xf8000001);
	msr.lo |= 0xf8000001;
	wrmsr(MMIO_CONF_BASE, msr);
	msr = rdmsr(MMIO_CONF_BASE);
	printk(BIOS_ERR, "c0010058 val %x:%x\n", msr.hi, msr.lo);

	msr = rdmsr(TOP_MEM);
	printk(BIOS_ERR, "TOP %x:%x\n", msr.hi,msr.lo);
	tomk = (msr.lo>>10)&TOP_MEM_MASK;
	printk(BIOS_DEBUG, "QEMU: cmos: %lu MiB RAM below 4G.\n", tomk / 1024);
	/* Report the memory regions. */
	ram_resource(dev, 10, 0, 640);
	ram_resource(dev, 11, 768, tomk - 768);
//	if (high)
//		ram_resource(dev, idx++, 4 * 1024 * 1024, high);
	db();
	print_func_exit();
}

struct chip_operations mainboard_amd_romepsp_ops = {
	.enable_dev = mainboard_amd_romepsp_enable,
};

static void mainboard_enable(struct device *dev)
{
	print_func_entry();
	printk(BIOS_INFO, "Mainboard " CONFIG_MAINBOARD_PART_NUMBER " Enable.\n");
	print_func_exit();
}


struct chip_operations mainboard_ops = {
	.enable_dev = mainboard_enable,
};
