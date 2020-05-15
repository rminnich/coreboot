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
#include "memory.h"

#include "acpi.h"

static void qemu_reserve_ports(struct device *dev, unsigned int idx,
			       unsigned int base, unsigned int size,
			       const char *name)
{
	print_func_entry();
	unsigned int end = base + size - 1;
	struct resource *res;

	printk(BIOS_DEBUG, "QEMU: reserve ioports 0x%04x-0x%04x [%s]\n",
	       base, end, name);
	res = new_resource(dev, idx);
	res->base = base;
	res->size = size;
	res->limit = 0xffff;
	res->flags = IORESOURCE_IO | IORESOURCE_FIXED | IORESOURCE_STORED |
		IORESOURCE_ASSIGNED;
	print_func_exit();
}

static void cpu_pci_domain_set_resources(struct device *dev)
{
	print_func_entry();
	assign_resources(dev->link_list);
	print_func_exit();
}

static void cpu_pci_domain_read_resources(struct device *dev)
{
	print_func_entry();
	u16 nbid   = pci_read_config16(pcidev_on_root(0x0, 0), PCI_DEVICE_ID);
	int i440fx = (nbid == 0x1237);
	int q35    = (nbid == 0x29c0);
	struct resource *res;
	unsigned long tomk = 0, high;
	int idx = 10;

	pci_domain_read_resources(dev);

	if (!tomk) {
		/* qemu older than 1.7, or reading etc/e820 failed. Fallback to cmos. */
		tomk = qemu_get_memory_size();
		high = qemu_get_high_memory_size();
		printk(BIOS_DEBUG, "QEMU: cmos: %lu MiB RAM below 4G.\n", tomk / 1024);
		printk(BIOS_DEBUG, "QEMU: cmos: %lu MiB RAM above 4G.\n", high / 1024);

		/* Report the memory regions. */
		ram_resource(dev, idx++, 0, 640);
		ram_resource(dev, idx++, 768, tomk - 768);
		if (high)
			ram_resource(dev, idx++, 4 * 1024 * 1024, high);
	}

	/* Reserve I/O ports used by QEMU */
	qemu_reserve_ports(dev, idx++, 0x0510, 0x02, "firmware-config");
	qemu_reserve_ports(dev, idx++, 0x5658, 0x01, "vmware-port");
	if (i440fx) {
		qemu_reserve_ports(dev, idx++, 0xae00, 0x10, "pci-hotplug");
		qemu_reserve_ports(dev, idx++, 0xaf00, 0x20, "cpu-hotplug");
		qemu_reserve_ports(dev, idx++, 0xafe0, 0x04, "piix4-gpe0");
	}
	if (inb(CONFIG_CONSOLE_QEMU_DEBUGCON_PORT) == 0xe9) {
		qemu_reserve_ports(dev, idx++, CONFIG_CONSOLE_QEMU_DEBUGCON_PORT, 1,
				   "debugcon");
	}

	/* A segment is legacy VGA region */
	mmio_resource(dev, idx++, 0xa0000 / KiB, (0xc0000 - 0xa0000) / KiB);

	/* C segment to 1MB is reserved RAM (low tables) */
	reserved_ram_resource(dev, idx++, 0xc0000 / KiB, (1 * MiB - 0xc0000) / KiB);

	if (q35 && ((tomk * 1024) < 0xb0000000)) {
		/*
		 * Reserve the region between top-of-ram and the
		 * mmconf xbar (ar 0xb0000000), so coreboot doesn't
		 * place pci bars there.  The region isn't declared as
		 * pci io window in the ACPI tables (\_SB.PCI0._CRS).
		 */
		res = new_resource(dev, idx++);
		res->base = tomk * 1024;
		res->size = 0xb0000000 - tomk * 1024;
		res->limit = 0xffffffff;
		res->flags = IORESOURCE_MEM | IORESOURCE_FIXED |
			IORESOURCE_STORED | IORESOURCE_ASSIGNED;
	}

	if (i440fx) {
		/* Reserve space for the IOAPIC.  This should be in
		 * the southbridge, but I couldn't tell which device
		 * to put it in. */
		res = new_resource(dev, 2);
		res->base = IO_APIC_ADDR;
		res->size = 0x100000UL;
		res->limit = 0xffffffffUL;
		res->flags = IORESOURCE_MEM | IORESOURCE_FIXED |
			IORESOURCE_STORED | IORESOURCE_ASSIGNED;
	}

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

#if 0 // CONFIG(GENERATE_SMBIOS_TABLES)
static int qemu_get_smbios_data16(int handle, unsigned long *current)
{
	print_func_entry();
	struct smbios_type16 *t = (struct smbios_type16 *)*current;
	int len = sizeof(struct smbios_type16);

	memset(t, 0, sizeof(struct smbios_type16));
	t->type = SMBIOS_PHYS_MEMORY_ARRAY;
	t->handle = handle;
	t->length = len - 2;
	t->location = MEMORY_ARRAY_LOCATION_SYSTEM_BOARD;
	t->use = MEMORY_ARRAY_USE_SYSTEM;
	t->memory_error_correction = MEMORY_ARRAY_ECC_NONE;
	t->maximum_capacity = qemu_get_memory_size();
	*current += len;
	print_func_exit();
	return len;
}

static int qemu_get_smbios_data17(int handle, int parent_handle, unsigned long *current)
{
	print_func_entry();
	struct smbios_type17 *t = (struct smbios_type17 *)*current;
	int len;

	memset(t, 0, sizeof(struct smbios_type17));
	t->type = SMBIOS_MEMORY_DEVICE;
	t->handle = handle;
	t->phys_memory_array_handle = parent_handle;
	t->length = sizeof(struct smbios_type17) - 2;
	t->size = qemu_get_memory_size() / 1024;
	t->data_width = 64;
	t->total_width = 64;
	t->form_factor = 9; /* DIMM */
	t->device_locator = smbios_add_string(t->eos, "Virtual");
	t->memory_type = 0x12; /* DDR */
	t->type_detail = 0x80; /* Synchronous */
	t->speed = 200;
	t->clock_speed = 200;
	t->manufacturer = smbios_add_string(t->eos, CONFIG_MAINBOARD_VENDOR);
	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	print_func_exit();
	return len;
}

static int qemu_get_smbios_data(struct device *dev, int *handle, unsigned long *current)
{
	print_func_entry();
	int len = -1;

	if (len != 0) {
		print_func_exit();
		return len;
	}

	print_func_exit();
	return len;
}
#endif

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
	printk(BIOS_INFO, "QEMU: max_cpus is %d\n", max_cpus);
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

