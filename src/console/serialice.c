//* SPDX-License-Identifier: GPL-2.0-only */

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

#include "serialice.h"

/* Accessor functions */

static void serialice_read_memory(void)
{
	u8 width;
	u32 addr;

	// Format:
	// *rm00000000.w
	addr = sio_get32();
	sio_getc(); // skip .
	width = sio_getc();

	sio_putc('\r');
	sio_putc('\n');

	switch (width) {
	case 'b':
		sio_put8(read8((void *)addr));
		break;
	case 'w':
		sio_put16(read16((void *)addr));
		break;
	case 'l':
		sio_put32(read32((void *)addr));
		break;
	}
}

static void serialice_write_memory(void)
{
	u8 width;
	u32 addr;
	u32 data;

	// Format:
	// *wm00000000.w=0000
	addr = sio_get32();
	sio_getc(); // skip .
	width = sio_getc();
	sio_getc(); // skip =

	switch (width) {
	case 'b':
		data = sio_get8();
		write8((void *)addr, (u8)data);
		break;
	case 'w':
		data = sio_get16();
		write16((void *)addr, (u16)data);
		break;
	case 'l':
		data = sio_get32();
		write32((void *)addr, (u32)data);
		break;
	}
}

static void serialice_read_io(void)
{
	u8 width;
	u16 port;

	// Format:
	// *ri0000.w
	port = sio_get16();
	sio_getc(); // skip .
	width = sio_getc();

	sio_putc('\r');
	sio_putc('\n');

	switch (width) {
	case 'b':
		sio_put8(inb(port));
		break;
	case 'w':
		sio_put16(inw(port));
		break;
	case 'l':
		sio_put32(inl(port));
		break;
	}
}

static void serialice_write_io(void)
{
	u8 width;
	u16 port;
	u32 data;

	// Format:
	// *wi0000.w=0000
	port = sio_get16();
	sio_getc(); // skip .
	width = sio_getc();
	sio_getc(); // skip =

	switch (width) {
	case 'b':
		data = sio_get8();
		outb((u8)data, port);
		break;
	case 'w':
		data = sio_get16();
		outw((u16)data, port);
		break;
	case 'l':
		data = sio_get32();
		outl((u32)data, port);
		break;
	}
}

typedef struct {
	u32 lo, hi;
} msrx_t;

static inline void rdmsrx(u32 index, u32 *lo, u32 *hi, u32 key)
{
	u32 l, h;
	__asm__ __volatile__("rdmsr" : "=a"(l), "=d"(h) : "c"(index), "D"(key));
	*lo = l;
	*hi = h;
}

static inline void wrmsrx(u32 index, u32 lo, u32 hi, u32 key)
{
	__asm__ __volatile__("wrmsr"
			     : /* No outputs */
			     : "c"(index), "a"(lo), "d"(hi), "D"(key));
}


static void serialice_read_msr(void)
{
	u32 addr, key, lo, hi;

	// Format:
	// *rc00000000.9c5a203a
	addr = sio_get32();
	sio_getc(); // skip .
	// Key is, I assume, for password-protected MSRs.
	key = sio_get32(); // key in %edi

	sio_putc('\r');
	sio_putc('\n');

	rdmsrx(addr, &lo, &hi, key);
	// msr = rdmsr(addr, key);
	sio_put32(hi);
	sio_putc('.');
	sio_put32(lo);
}

static void serialice_write_msr(void)
{
	u32 addr, key, lo, hi;

	// Format:
	// *wc00000000.9c5a203a=00000000.00000000
	addr = sio_get32();
	sio_getc();	// skip .
	key = sio_get32(); // read key in %edi
	sio_getc();	// skip =
	hi = sio_get32();
	sio_getc(); // skip .
	lo = sio_get32();

	wrmsrx(addr, lo, hi, key);
}

/* CPUID functions */

static inline u32 xcpuid_eax(u32 op, u32 op2)
{
	u32 eax;

	__asm__("cpuid" : "=a"(eax) : "a"(op), "c"(op2) : "ebx", "edx");
	return eax;
}

static inline u32 xcpuid_ebx(u32 op, u32 op2)
{
	u32 ebx;

	__asm__("cpuid" : "=b"(ebx) : "a"(op), "c"(op2) : "edx");
	return ebx;
}

static inline u32 xcpuid_ecx(u32 op, u32 op2)
{
	u32 ecx;

	__asm__("cpuid" : "=c"(ecx) : "a"(op), "c"(op2) : "ebx", "edx");
	return ecx;
}

static inline u32 xcpuid_edx(u32 op, u32 op2)
{
	u32 edx;

	__asm__("cpuid" : "=d"(edx) : "a"(op), "c"(op2) : "ebx");
	return edx;
}

static void serialice_cpuinfo(void)
{
	u32 eax, ecx;
	u32 reg32;

	// Format:
	//    --EAX--- --ECX---
	// *ci00000000.00000000
	eax = sio_get32();
	sio_getc(); // skip .
	ecx = sio_get32();

	sio_putc('\r');
	sio_putc('\n');

	/* This code looks quite crappy but this way we don't
	 * have to worry about running out of registers if we
	 * occupy eax, ebx, ecx, edx at the same time
	 */
	reg32 = xcpuid_eax(eax, ecx);
	sio_put32(reg32);
	sio_putc('.');

	reg32 = xcpuid_ebx(eax, ecx);
	sio_put32(reg32);
	sio_putc('.');

	reg32 = xcpuid_ecx(eax, ecx);
	sio_put32(reg32);
	sio_putc('.');

	reg32 = xcpuid_edx(eax, ecx);
	sio_put32(reg32);
}

static void serialice_mainboard(void)
{
	int i;

	sio_putc('\r');
	sio_putc('\n');
	for (i = 0; i < strlen(CONFIG_MAINBOARD_SMBIOS_PRODUCT_NAME) && i < 33; i++)
		sio_putc(CONFIG_MAINBOARD_SMBIOS_PRODUCT_NAME[i]);
	for (; i < 33; i++)
		sio_putc(' ');
	sio_putc('\r');
	sio_putc('\n');
}

static void serialice_version(void)
{
	sio_putstring("\nSerialICE embedded in coreboot\n");
}

#define CMD(a, b) (((u16)(a) << 8) | (b))
void ice(void)
{
	int done = 0;

	serialice_version();

	while (!done) {
		u16 c;
		sio_putstring("\n> ");

		c = sio_getc();
		if (c != '*')
			continue;

		c = sio_getc() << 8;
		c |= sio_getc();

		switch (c) {
		case CMD('r', 'm'): // Read Memory *rm
			serialice_read_memory();
			break;
		case CMD('w', 'm'): // Write Memory *wm
			serialice_write_memory();
			break;
		case CMD('r', 'i'): // Read IO *ri
			serialice_read_io();
			break;
		case CMD('w', 'i'): // Write IO *wi
			serialice_write_io();
			break;
		case CMD('r', 'c'): // Read CPU MSR *rc
			serialice_read_msr();
			break;
		case CMD('w', 'c'): // Write CPU MSR *wc
			serialice_write_msr();
			break;
		case CMD('c', 'i'): // Read CPUID *ci
			serialice_cpuinfo();
			break;
		case CMD('m', 'b'): // Read mainboard type *mb
			serialice_mainboard();
			break;
		case CMD('v', 'i'): // Read version info *vi
			serialice_version();
			break;
		case CMD('e', 'x'): // Done.
			sio_putstring("Returning to coreboot\n");
			return;
		default:
			sio_putstring("ERROR\n");
			break;
		}
	}
}
