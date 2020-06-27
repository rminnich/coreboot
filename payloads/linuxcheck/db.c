/* SPDX-License-Identifier: GPL-2.0-only */

#include <libpayload-config.h>
#include <libpayload.h>
#include <arch/apic.h>
#include <exception.h>
#include <arch/msr.h>
#include "linuxcheck.h"

struct cmd {
	const char *name;
	const char *usage;
	int nargs;
	void (*f)(int nargc, uint64_t *args);
};

#ifdef later
static void do_cpuid(int argc, uint64_t *args)
{
	uint32_t v;
	v = cpuid_eax(args[0]);
	printf( "EAX: %#x,", v);
	v = cpuid_ebx(args[0]);
	printf( "EBX: %#x,", v);
	v = cpuid_ecx(args[0]);
	printf( "ECX: %#x,", v);
	v = cpuid_edx(args[0]);
	printf( "EDX: %#x\r\n", v);
}
#endif
static void do_rdmsr(int argc, uint64_t *args)
{
	uint32_t hi, lo;
	uint32_t a = (uint32_t) args[0];
	rdmsr(a, lo, hi);
	printf( "rdmsr %#x:%#x:%#x\r\n", a, hi, lo);
}

static void do_irq(int argc, uint64_t *args)
{
	uint32_t a = (uint32_t) args[0];
	if (a) {
		printf("sti....");
		__asm__ __volatile__("sti\n");
		printf("\n");
	} else {
		printf("cli....");
		__asm__ __volatile__("cli\n");
		printf("\n");
	}
}

#if ENV_RAMSTAGE
static int readsnm(uint32_t start, int size, uint32_t *dat)
{
	struct device *dev = dev_find_device(0x1022, 0x1480, NULL);
	if (! dev) {
		printf( "NO 0x1022 0x1480\n");
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
		printf( "smn read %#x:%#x\r\n", a, dat[0]);
}

static void do_sw(int argc, uint64_t *args)
{
	uint32_t a = (uint32_t) args[0];
	uint32_t v = (uint32_t) args[1];
	struct device *dev = dev_find_device(0x1022, 0x1480, NULL);
	if (! dev) {
		printf( "NO 0x1022 0x1480\n");
		return;
	}

	pci_write_config32(dev, 0xb8, a);
	pci_write_config32(dev, 0xbc, v);
	printf( "smn write %#x:%#x\r\n", a, v);
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
	printf( "mem %p:%#x; ", a, *a);
	*a = v;
	printf( ":%#x, n", v);
	printf( "result is mem %#x\r\n", *a);
}

static void do_mem(int argc, uint64_t *args)
{
	uint32_t v;
	uint32_t *a = (uint32_t*) (uint32_t)(args[0]);
	v = *a;
	printf( "mem %p:%#x\r\n", a, v);
}

static void do_flags(int argc, uint64_t *args)
{
	uint32_t flags(void);
	printf("Flags %#x\n", flags());
}

static void do_xmem(int argc, uint64_t *args)
{
	uint32_t *a = (uint32_t*) (uint32_t)(args[0]);
	hexdump(a, args[1]);
}

static void do_wrmsr(int argc, uint64_t *args)
{
	uint32_t msr = (uint32_t) args[0];
	uint32_t lo, hi;
	rdmsr(msr, lo, hi);
	printf( "wrmsr before %#x:%#x:%#x\r\n", msr, hi, lo);
	lo = (uint32_t) args[1];
	hi = (uint32_t)(args[1]>>32);
	wrmsr(msr, lo, hi);
	rdmsr(msr, lo, hi);
	printf( "wrmsr after %#x:%#x:%#x\r\n", msr, hi, lo);
}

static void do_inl(int argc, uint64_t *args)
{
	uint16_t a = (uint16_t)args[0];
	uint32_t v = inl(a);
	printf( "%#x: %#x\r\n", a, v);
}

static void do_inw(int argc, uint64_t *args)
{
	uint16_t a = (uint16_t)args[0];
	uint16_t v = inw(a);
	printf( "%#x: %#x\r\n", a, v);
}

static void do_inb(int argc, uint64_t *args)
{
	uint16_t a = (uint16_t)args[0];
	uint8_t v = inb(a);
	printf( "%#x: %#x\r\n", a, v);
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

	printf( "Convert '%s'\r\n", s);
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

void apic_start_delay(unsigned int usec);
void do_timer(int argc, uint64_t *args)
{
	uint32_t v = args[0];
	printf("Start timer for %d seconds", v);
	apic_start_delay(v);
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
//	{"cpuid", "cpuid_get_cpuid from coreboot2", 1, do_cpuid,},
	{"msr", "read an MSR", 1, do_rdmsr,},
	{"wmsr", "write an MSR", 2, do_wrmsr,},
	{"mem", "read mem", 1, do_mem,},
	{"xmem", "x mem", 2, do_xmem,},
	{"wmem", "w mem", 2, do_wmem,},
	{"irq", "enable.disable irq", 1, do_irq,},
	//{"sr", "smn read", 1, do_sr,},
	//{"sw", "smn_write", 2, do_sw},
	//{"xs", "dump snm", 2, do_xs},
	{"timer", "start apic timer", 1, do_timer},
	{"flags", "show flags", 0, do_flags},
	{"ice", "serial ice", 0, ice},
};
	struct cmd *c;
	uint8_t b;
	int l, nargs;
	static char line[96]; // not 80
	static char *parms[NCMD];
	static uint64_t args[NCMD];
	while (1) {
		extern volatile uint8_t timer_waiting;

		l = nargs = 0;
		memset(line, 0, sizeof(line));
		memset(parms, 0, sizeof(parms));
		memset(args, 0, sizeof(args));
		printf( "DB(timer_waiting=%d>", timer_waiting);
		while (l < sizeof(line)) {
			b = getchar();
			if (!b) {
				continue;
			}
			if (b == 4) {
				return;
			}
			putchar(b);
			if ((b == '\n') || (b == '\r')) {
				printf( "\r\n");
				break;
			}
			line[l] = b;
			l++;
		}
		printf( "line is %s\r\n", line);
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
				printf( "%s: %s, %d args\r\n", cmds[i].name, cmds[i].usage, cmds[i].nargs);
			}
			continue;
		}

		printf( "nargs %d\r\n", nargs);
		for(int i = 0; i < nargs; i++)
			printf( "%d: '%s'\r\n", i, parms[i]);

		c = NULL;
		for (int i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
			printf( "Check '%s' against '%s'\r\n", parms[0], cmds[i].name);
			if (! strcmp(parms[0], cmds[i].name)) {
				printf( "Found\r\n");
				c = &cmds[i];
				break;
			}
		}
		if (! c) {
			printf( "%s: not found", parms[0]);
			printf( "\r\n");
			for (int i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
				printf( "%s: %s, %d args\r\n", cmds[i].name, cmds[i].usage, cmds[i].nargs);
			}
			continue;
		}
		if (nargs-1 != c->nargs){
			printf( "Usage: %s %d args\r\n", c->name, c->nargs);
			continue;
		}
		for(int i = 1; i < nargs; i++) {
			args[i-1] = string2bin(parms[i]);
			printf( "%d: %#llx\r\n", i, args[i-1]);
		}
		c->f(nargs-1, args);
	}
}
