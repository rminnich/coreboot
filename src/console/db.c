/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <console/uart.h>
#include <string.h>
#include <ctype.h>
#include <arch/io.h>
#include <cpu/cpu.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/msr.h>
#include <lib.h>

#include <device/cardbus.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>
#include <device/pcix.h>
#include <device/pciexp.h>

struct cmd {
	const char *name;
	const char *usage;
	int nargs;
	void (*f)(int nargc, uint64_t *args);
};

static void do_drivers(int argc, uint64_t *args)
{
	struct pci_driver *driver;
	printk(BIOS_ERR, "There are %ld drivers\n", &_epci_drivers[0] - &_pci_drivers[0]);
	for (driver = &_pci_drivers[0]; driver != &_epci_drivers[0]; driver++) {
		printk(BIOS_ERR, "[%04x/%04x] %sops\n",
		       driver->vendor, driver->device,
		       (driver->ops->scan_bus ? "bus " : ""));
		}
}

static void do_cpuid(int argc, uint64_t *args)
{
	uint32_t v;
	v = cpuid_eax(args[0]);
	printk(BIOS_ERR, "EAX: %#x,", v);
	v = cpuid_ebx(args[0]);
	printk(BIOS_ERR, "EBX: %#x,", v);
	v = cpuid_ecx(args[0]);
	printk(BIOS_ERR, "ECX: %#x,", v);
	v = cpuid_edx(args[0]);
	printk(BIOS_ERR, "EDX: %#x\r\n", v);
}

static void do_rdmsr(int argc, uint64_t *args)
{
	msr_t msr;
	uint32_t a = (uint32_t) args[0];
	msr = rdmsr(a);
	printk(BIOS_ERR, "rdmsr %#x:%#x:%#x\r\n", a, msr.hi, msr.lo);
}

static void do_irq(int argc, uint64_t *args)
{
	uint32_t a = (uint32_t) args[0];
	if (a) {
		printk(BIOS_SPEW, "sti....");
		__asm__ __volatile__("sti\n");
		printk(BIOS_SPEW, "\n");
	} else {
		printk(BIOS_SPEW, "cli....");
		__asm__ __volatile__("cli\n");
		printk(BIOS_SPEW, "\n");
	}
}

#if ENV_RAMSTAGE
static int readsnm(uint32_t start, int size, uint32_t *dat)
{
	struct device *dev = dev_find_device(0x1022, 0x1480, NULL);
	if (! dev) {
		printk(BIOS_ERR, "NO 0x1022 0x1480\n");
		return - 1;
	}

	for (int i = 0; i < size; i++) {
		pci_write_config32(dev, 0xb8, start+i);
		dat[i] = pci_read_config32(dev, 0xbc);
	}
	return size;
}

static void do_sr(int argc, uint64_t *args)
{
	uint32_t dat[1];
	uint32_t a = (uint32_t) args[0];
	if (readsnm(a, 1, dat) > 0)
		printk(BIOS_ERR, "smn read %#x:%#x\r\n", a, dat[0]);
}

static void do_sw(int argc, uint64_t *args)
{
	uint32_t a = (uint32_t) args[0];
	uint32_t v = (uint32_t) args[1];
	struct device *dev = dev_find_device(0x1022, 0x1480, NULL);
	if (! dev) {
		printk(BIOS_ERR, "NO 0x1022 0x1480\n");
		return;
	}

	pci_write_config32(dev, 0xb8, a);
	pci_write_config32(dev, 0xbc, v);
	printk(BIOS_ERR, "smn write %#x:%#x\r\n", a, v);
}

static void do_xs(int argc, uint64_t *args)
{
	static uint32_t dat[4096];
	uint32_t a = (uint32_t)(args[0]);
	uint32_t s = (uint32_t)(args[1]);
	if (s > sizeof(dat)/4)
		s = sizeof(dat) / 4;
	if (readsnm(a, s, dat) > 0 )
		hexdump(dat, s);
}

#endif
static void do_wmem(int argc, uint64_t *args)
{
	uint32_t *a = (uint32_t*) (uint32_t)(args[0]);
	uint32_t v = (uint32_t)(args[1]);
	printk(BIOS_ERR, "mem %p:%#x; ", a, *a);
	*a = v;
	printk(BIOS_ERR, ":%#x, n", v);
	printk(BIOS_ERR, "result is mem %#x\r\n", *a);
}

static void do_mem(int argc, uint64_t *args)
{
	uint32_t v;
	uint32_t *a = (uint32_t*) (uint32_t)(args[0]);
	v = *a;
	printk(BIOS_ERR, "mem %p:%#x\r\n", a, v);
}

static void do_xmem(int argc, uint64_t *args)
{
	uint32_t *a = (uint32_t*) (uint32_t)(args[0]);
	hexdump(a, args[1]);
}

static void do_wrmsr(int argc, uint64_t *args)
{
	msr_t msr;
	uint32_t a = (uint32_t) args[0];
	uint32_t lo = (uint32_t) args[1], hi = (uint32_t)(args[1]>>32);
	msr = rdmsr(a);
	printk(BIOS_ERR, "wrmsr before %#x:%#x:%#x\r\n", a, msr.hi, msr.lo);
	msr.lo = lo;
	msr.hi = hi;
	wrmsr(a, msr);
	msr = rdmsr(a);
	printk(BIOS_ERR, "wrmsr after %#x:%#x:%#x\r\n", a, msr.hi, msr.lo);
}

static void do_inl(int argc, uint64_t *args)
{
	uint16_t a = (uint16_t)args[0];
	uint32_t v = inl(a);
	printk(BIOS_ERR, "%#x: %#x\r\n", a, v);
}

static void do_inw(int argc, uint64_t *args)
{
	uint16_t a = (uint16_t)args[0];
	uint16_t v = inw(a);
	printk(BIOS_ERR, "%#x: %#x\r\n", a, v);
}

static void do_inb(int argc, uint64_t *args)
{
	uint16_t a = (uint16_t)args[0];
	uint8_t v = inb(a);
	printk(BIOS_ERR, "%#x: %#x\r\n", a, v);
}

static void do_outl(int argc, uint64_t *args)
{
	uint16_t a = (uint16_t)args[0];
	uint32_t v = args[1];
	outl(v, a);
}

static void do_outw(int argc, uint64_t *args)
{
	uint16_t a = (uint16_t)args[0];
	uint16_t v = args[1];
	outl(v, a);
}

static void do_outb(int argc, uint64_t *args)
{
	uint16_t a = (uint16_t)args[0];
	uint8_t v = args[1];
	outb(v, a);
}
static uint64_t string2bin(char *s)
{
	uint64_t val = 0;

	printk(BIOS_ERR, "Convert '%s'\r\n", s);
	for(int i = 0; i < strlen(s); i++) {
		uint8_t c = s[i];
		val <<= 4;
		if (isdigit(c)) {
			val |= c - '0';
		} else {
			val |= 10 + (tolower(c)) - 'a';
		}
	}
	return val;
}

#define NCMD 8
/* this code is intended to be callable at any time, and resume at any time */
/* Tawk to me */
void db(void)
{
	struct cmd cmds[] = {
	{"inl", "inl address", 1, do_inl},
	{"inw", "inw address", 1, do_inw,},
	{"inb", "inb address", 1, do_inb,},
	{"outl", "outl address data", 2, do_outl,},
	{"outw", "outw address data", 2, do_outw,},
	{"outb", "outb address data", 2, do_outb,},
	{"cpuid", "cpuid_get_cpuid from coreboot2", 1, do_cpuid,},
	{"msr", "read an MSR", 1, do_rdmsr,},
	{"wmsr", "write an MSR", 2, do_wrmsr,},
	{"mem", "read mem", 1, do_mem,},
	{"xmem", "x mem", 2, do_xmem,},
	{"wmem", "w mem", 2, do_wmem,},
	{"dr", "show drivers", 0, do_drivers,},
	{"irq", "enable.disable irq", 1, do_irq,},
#if ENV_RAMSTAGE
	{"sr", "smn read", 1, do_sr,},
	{"sw", "smn_write", 2, do_sw},
	{"xs", "dump snm", 2, do_xs},
#endif
};
	struct cmd *c;
	uint8_t b;
	int l, ll, nargs;
	static char line[96]; // not 80
	static char *parms[NCMD];
	static uint64_t args[NCMD];
	while (1) {
		ll = l = nargs = 0;
		memset(line, 0, sizeof(line));
		memset(parms, 0, sizeof(parms));
		memset(args, 0, sizeof(args));
		printk(BIOS_ERR, "DB>");
		while (l < sizeof(line)) {
			b = uart_rx_byte(0);
			if (!b) {
				continue;
			}
			if (b == 4) {
				return;
			}
			uart_tx_byte(0, b);
			if ((b == '\n') || (b == '\r')) {
				printk(BIOS_ERR, "\r\n");
				break;
			}
			line[l] = b;
			l++;
		}
		printk(BIOS_ERR, "line is %s\r\n", line);
		for(int i = 0; i < l; i++) {
			if (isspace(line[i])) {
				line[i] = 0;
				continue;
			}
			parms[nargs] = &line[i];
			nargs++;
			for(;(i < l) && (! isspace(line[i])); i++)
					;
			if (isspace(line[i]))
				line[i] = 0;
		}
		if (nargs == 0) {
			for (int i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
				printk(BIOS_ERR, "%s: %s, %d args\r\n", cmds[i].name, cmds[i].usage, cmds[i].nargs);
			}
			continue;
		}

		printk(BIOS_ERR, "nargs %d\r\n", nargs);
		for(int i = 0; i < nargs; i++)
			printk(BIOS_ERR, "%d: '%s'\r\n", i, parms[i]);

		c = NULL;
		for (int i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
			printk(BIOS_ERR, "Check '%s' against '%s'\r\n", parms[0], cmds[i].name);
			if (! strcmp(parms[0], cmds[i].name)) {
				printk(BIOS_ERR, "Found\r\n");
				c = &cmds[i];
				break;
			}
		}
		if (! c) {
			printk(BIOS_ERR, "%s: not found", parms[0]);
			printk(BIOS_ERR, "\r\n");
			for (int i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
				printk(BIOS_ERR, "%s: %s, %d args\r\n", cmds[i].name, cmds[i].usage, cmds[i].nargs);
			}
			continue;
		}
		if (nargs-1 != c->nargs){
			printk(BIOS_ERR, "Usage: %s %d args\r\n", c->name, c->nargs);
			continue;
		}
		for(int i = 1; i < nargs; i++) {
			args[i-1] = string2bin(parms[i]);
			printk(BIOS_ERR, "%d: %#llx\r\n", i, args[i-1]);
		}
		c->f(nargs-1, args);
	}
}
