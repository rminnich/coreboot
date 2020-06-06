/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/cpu.h>
#include <acpi/acpi.h>
#include <cpu/x86/cache.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/mtrr.h>
#include <console/uart.h>
#include <cbmem.h>
#include <commonlib/helpers.h>
#include <console/console.h>
#include <program_loading.h>
#include <elog.h>
#include <soc/romstage.h>
#include <soc/mtrr.h>
#include <types.h>
#include "chip.h"

asmlinkage void car_stage_entry(void)
{
	//int s3_resume;

	post_code(0x40);
	console_init();

	post_code(0x41);
	//s3_resume = acpi_s3_resume_allowed() && acpi_is_wakeup_s3();
	//mainboard_romstage_entry_s3(s3_resume);
	//elog_boot_notify(s3_resume);

	post_code(0x42);
	u32 val = cpuid_eax(1);
	printk(BIOS_DEBUG, "Family_Model: %08x\n", val);

	post_code(0x43);
	romepsp_save_mtrrs();

	post_code(0x44);

	post_code(0x45);

	post_code(0x46);
	run_ramstage();

	post_code(0x50); /* Should never see this post code. */
}
