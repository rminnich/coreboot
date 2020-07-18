/* SPDX-License-Identifier: GPL-2.0-only */

#include <stdint.h>
#include <symbols.h>
#include <bootblock_common.h>
#include <console/console.h>
#include <cpu/x86/cache.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/msr.h>
#include <cpu/x86/mtrr.h>
#include <cpu/amd/mtrr.h>
#include <soc/southbridge.h>
#include <soc/i2c.h>
#include <amdblocks/amd_pci_mmconf.h>
#include <acpi/acpi.h>
#include <security/vboot/symbols.h>

/* vboot includes directory may not be in include path if vboot is not enabled */
#if CONFIG(VBOOT_STARTS_BEFORE_BOOTBLOCK)
#include <2struct.h>
#endif

#include <arch/io.h>
asmlinkage void bootblock_resume_entry(void);

/* PSP performs the memory training and setting up DRAM map prior to x86 cores
   being released. Honor TOP_MEM and set up caching from 0 til TOP_MEM. Likewise,
   route lower memory addresses covered by fixed MTRRs to DRAM except for
   0xa0000-0xc0000 . */
static void set_caching(void)
{
	msr_t top_mem;
	msr_t sys_cfg;
	msr_t mtrr_def_type;
	msr_t fixed_mtrr_ram;
	msr_t fixed_mtrr_mmio;
	struct var_mtrr_context mtrr_ctx;

	outb(0xbe, 0x80);
	var_mtrr_context_init(&mtrr_ctx, NULL);
	outb(0xbf, 0x80);
	top_mem = rdmsr(TOP_MEM);
	outb(0xc0, 0x80);
	/* Enable RdDram and WrDram attributes in fixed MTRRs. */
	sys_cfg = rdmsr(SYSCFG_MSR);
	outb(0xc1, 0x80);
	sys_cfg.lo |= SYSCFG_MSR_MtrrFixDramModEn;
	outb(0xc2, 0x80);

	/* Fixed MTRR constants. */
	fixed_mtrr_ram.lo = fixed_mtrr_ram.hi =
		((MTRR_TYPE_WRBACK | MTRR_READ_MEM | MTRR_WRITE_MEM) <<  0) |
		((MTRR_TYPE_WRBACK | MTRR_READ_MEM | MTRR_WRITE_MEM) <<  8) |
		((MTRR_TYPE_WRBACK | MTRR_READ_MEM | MTRR_WRITE_MEM) << 16) |
		((MTRR_TYPE_WRBACK | MTRR_READ_MEM | MTRR_WRITE_MEM) << 24);
	fixed_mtrr_mmio.lo = fixed_mtrr_mmio.hi =
		((MTRR_TYPE_UNCACHEABLE) <<  0) |
		((MTRR_TYPE_UNCACHEABLE) <<  8) |
		((MTRR_TYPE_UNCACHEABLE) << 16) |
		((MTRR_TYPE_UNCACHEABLE) << 24);

	/* Prep default MTRR type. */
	mtrr_def_type = rdmsr(MTRR_DEF_TYPE_MSR);
	mtrr_def_type.lo &= ~MTRR_DEF_TYPE_MASK;
	mtrr_def_type.lo |= MTRR_TYPE_UNCACHEABLE;
	mtrr_def_type.lo |= MTRR_DEF_TYPE_EN | MTRR_DEF_TYPE_FIX_EN;

	disable_cache();
	outb(0xc3, 0x80);

	wrmsr(SYSCFG_MSR, sys_cfg);
	outb(0xc4, 0x80);

	clear_all_var_mtrr();
	outb(0xc5, 0x80);

	var_mtrr_set(&mtrr_ctx, 0, ALIGN_DOWN(top_mem.lo, 8*MiB), MTRR_TYPE_WRBACK);
	var_mtrr_set(&mtrr_ctx, FLASH_BASE_ADDR, CONFIG_ROM_SIZE, MTRR_TYPE_WRPROT);
	outb(0xc6, 0x80);

	/* Set up RAM caching for everything below 1MiB except for 0xa0000-0xc0000 . */
	wrmsr(MTRR_FIX_64K_00000, fixed_mtrr_ram);
	wrmsr(MTRR_FIX_16K_80000, fixed_mtrr_ram);
	wrmsr(MTRR_FIX_16K_A0000, fixed_mtrr_mmio);
	wrmsr(MTRR_FIX_4K_C0000, fixed_mtrr_ram);
	wrmsr(MTRR_FIX_4K_C8000, fixed_mtrr_ram);
	wrmsr(MTRR_FIX_4K_D0000, fixed_mtrr_ram);
	wrmsr(MTRR_FIX_4K_D8000, fixed_mtrr_ram);
	wrmsr(MTRR_FIX_4K_E0000, fixed_mtrr_ram);
	wrmsr(MTRR_FIX_4K_E8000, fixed_mtrr_ram);
	wrmsr(MTRR_FIX_4K_F0000, fixed_mtrr_ram);
	wrmsr(MTRR_FIX_4K_F8000, fixed_mtrr_ram);

	outb(0xc7, 0x80);
	wrmsr(MTRR_DEF_TYPE_MSR, mtrr_def_type);
	outb(0xc9, 0x80);

	/* Enable Fixed and Variable MTRRs. */
	sys_cfg.lo |= SYSCFG_MSR_MtrrFixDramEn | SYSCFG_MSR_MtrrVarDramEn;
	sys_cfg.lo |= SYSCFG_MSR_TOM2En | SYSCFG_MSR_TOM2WB;
	/* AGESA currently expects SYSCFG_MSR_MtrrFixDramModEn to be set. Once
	   MP init happens in coreboot proper it can be knocked down. */
	wrmsr(SYSCFG_MSR, sys_cfg);
	outb(0xca, 0x80);

	enable_cache();
	outb(0xcb, 0x80);
}

static void write_resume_eip(void)
{
	msr_t s3_resume_entry = {
		.hi = (uint64_t)(uintptr_t)bootblock_resume_entry >> 32,
		.lo = (uintptr_t)bootblock_resume_entry & 0xffffffff,
	};

	/*
	 * Writing to the EIP register can only be done once, otherwise a fault is triggered.
	 * When this register is written, it will trigger the microcode to stash the CPU state
	 * (crX , mtrrs, registers, etc) into the CC6 save area. On resume, the state will be
	 * restored and execution will continue at the EIP.
	 */
	if (!acpi_is_wakeup_s3())
		wrmsr(S3_RESUME_EIP_MSR, s3_resume_entry);
}

asmlinkage void bootblock_c_entry(uint64_t base_timestamp)
{
	outb(0xaa, 0x80);
	outb(0xa3, 0x80);
	set_caching();
	outb(0xa4, 0x80);
	if (0) write_resume_eip();
	outb(0xa5, 0x80);
	enable_pci_mmconf();
	outb(0xa6, 0x80);

	bootblock_main_with_basetime(base_timestamp);
	outb(0xa7, 0x80);
	while (1);
}

void bootblock_soc_early_init(void)
{
	outb(0xa8, 0x80);
	fch_pre_init();
	outb(0xa9, 0x80);
}

void bootblock_soc_init(void)
{
	u32 val = cpuid_eax(1);
	printk(BIOS_DEBUG, "Family_Model: %08x\n", val);

#if CONFIG(VBOOT_STARTS_BEFORE_BOOTBLOCK)
	if (*(uint32_t *)_vboot2_work != VB2_SHARED_DATA_MAGIC) {
		printk(BIOS_ERR, "ERROR: VBOOT workbuf not valid.\n");

		printk(BIOS_DEBUG, "Signature: %#08x\n", *(uint32_t *)_vboot2_work);

		die("Halting.\n");
	}
#endif

	fch_early_init();
}
