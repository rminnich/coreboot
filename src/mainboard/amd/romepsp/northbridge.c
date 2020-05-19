/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <cpu/cpu.h>
#include <cpu/x86/lapic_def.h>
#include <arch/io.h>
#include <device/pci_def.h>
#include <device/pci_ops.h>
#include <arch/ioapic.h>
#include <stdint.h>
#include <device/device.h>
#include <reset.h>
#include <stdlib.h>
#include <string.h>
#include <smbios.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/msr.h>
#include "memory.h"

#include "acpi.h"

#if 0
static void qemu_reserve_ports(struct device *dev, unsigned int idx,
			       unsigned int base, unsigned int size,
			       const char *name)
{
	print_func_entry();
	unsigned int end = base + size - 1;
	struct resource *res;

	printk(BIOS_DEBUG, "RomePSP: NOT reserve ioports 0x%04x-0x%04x [%s]\n",
	       base, end, name);
	if (false) {
		res = new_resource(dev, idx);
		res->base = base;
		res->size = size;
		res->limit = 0xffff;
		res->flags = IORESOURCE_IO | IORESOURCE_FIXED | IORESOURCE_STORED |
			IORESOURCE_ASSIGNED;
	}
	print_func_exit();
}
#endif

static void cpu_pci_domain_set_resources(struct device *dev)
{
	print_func_entry();
	printk(BIOS_ERR, "###########################################################\n");
	assign_resources(dev->link_list);
	print_func_exit();
}

/* these are in mtrr.h and I'm not ready for that. */
#define TOP_MEM		0xC001001Aul
#define TOP_MEM2	0xC001001Dul

#define TOP_MEM_MASK			0x007fffff
#define TOP_MEM_MASK_KB			(TOP_MEM_MASK >> 10)

static void cpu_pci_domain_read_resources(struct device *dev)
{
	msr_t msr;

	print_func_entry();
	printk(BIOS_ERR, "=====================================================\n");
	struct resource *res;
	unsigned long tomk = 0, high = 0;
	int idx = 10;

	pci_domain_read_resources(dev);

	msr = rdmsr(TOP_MEM);
	printk(BIOS_ERR, "TOP %x:%x\n", msr.hi,msr.lo);
	tomk = (msr.lo>>10)&TOP_MEM_MASK;
	printk(BIOS_DEBUG, "QEMU: cmos: %lu MiB RAM below 4G.\n", tomk / 1024);
//		printk(BIOS_DEBUG, "QEMU: cmos: %lu MiB RAM above 4G.\n", high / 1024);

		/* Report the memory regions. */
		ram_resource(dev, idx++, 0, 640);
		ram_resource(dev, idx++, 768, tomk - 768);
		if (high)
			ram_resource(dev, idx++, 4 * 1024 * 1024, high);

	/* A segment is legacy VGA region */
	mmio_resource(dev, idx++, 0xa0000 / KiB, (0xc0000 - 0xa0000) / KiB);

	/* C segment to 1MB is reserved RAM (low tables) */
	reserved_ram_resource(dev, idx++, 0xc0000 / KiB, (1 * MiB - 0xc0000) / KiB);

	/* Reserve space for the LAPIC.  There's one in every processor, but
	 * the space only needs to be reserved once, so we do it here. */
	res = new_resource(dev, 3);
	res->base = LOCAL_APIC_ADDR;
	res->size = 0x10000UL;
	res->limit = 0xffffffffUL;
	res->flags = IORESOURCE_MEM | IORESOURCE_FIXED | IORESOURCE_STORED |
		     IORESOURCE_ASSIGNED;
	print_func_exit();
}

#if CONFIG(HAVE_ACPI_TABLES)
static const char *qemu_acpi_name(const struct device *dev)
{
	print_func_entry();
	if (dev->path.type == DEVICE_PATH_DOMAIN) {
		print_func_exit();
		return "PCI0";
	}

	if (dev->path.type != DEVICE_PATH_PCI || dev->bus->secondary != 0) {
		print_func_exit();
		return NULL;
	}

	print_func_exit();
	return NULL;
}
#endif

static struct device_operations pci_domain_ops = {
	.read_resources		= cpu_pci_domain_read_resources,
	.set_resources		= cpu_pci_domain_set_resources,
	.scan_bus		= pci_domain_scan_bus,
#if 0 && CONFIG(GENERATE_SMBIOS_TABLES)
	.get_smbios_data	= qemu_get_smbios_data,
#endif
#if CONFIG(HAVE_ACPI_TABLES)
	.acpi_name		= qemu_acpi_name,
#endif
};

static void cpu_bus_init(struct device *dev)
{
	print_func_entry();
	initialize_cpus(dev->link_list);
	print_func_exit();
}

static void cpu_bus_scan(struct device *bus)
{
	print_func_entry();
	int max_cpus = 256;
	struct device *cpu;
	int i;

	if (max_cpus < 0) {
		print_func_exit();
		return;
	}

	/*
	 * TODO: This only handles the simple "qemu -smp $nr" case
	 * correctly.  qemu also allows to specify the number of
	 * cores, threads & sockets.
	 */
	printk(BIOS_INFO, "RomePSP: max_cpus is %d\n", max_cpus);
	for (i = 0; i < max_cpus; i++) {
		cpu = add_cpu_device(bus->link_list, i, 1);
		if (cpu)
			set_cpu_topology(cpu, 1, 0, i, 0);
	}
	print_func_exit();
}

static struct device_operations cpu_bus_ops = {
	.read_resources   = noop_read_resources,
	.set_resources    = noop_set_resources,
	.init             = cpu_bus_init,
	.scan_bus         = cpu_bus_scan,
};

static void northbridge_enable(struct device *dev)
{
	print_func_entry();
	/* Set the operations if it is a special bus type */
	if (dev->path.type == DEVICE_PATH_DOMAIN) {
		dev->ops = &pci_domain_ops;
	} else if (dev->path.type == DEVICE_PATH_CPU_CLUSTER) {
		dev->ops = &cpu_bus_ops;
	}
	print_func_exit();
}

struct chip_operations mainboard_emulation_qemu_q35_ops = {
	CHIP_NAME("AMD PSP Northbridge")
	.enable_dev = northbridge_enable,
};

void do_board_reset(void)
{
	print_func_entry();
	die("reset");
	print_func_exit();
}

