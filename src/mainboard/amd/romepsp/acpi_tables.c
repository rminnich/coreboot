/* SPDX-License-Identifier: GPL-2.0-only */

#include <types.h>
#include <string.h>
#include <acpi/acpi.h>
#include <arch/ioapic.h>
#include <arch/smp/mpspec.h>
#include <device/device.h>
#include <device/pci_ops.h>
#include <version.h>

#include "acpi.h"
//#include <southbridge/intel/i82801ix/nvs.h>

#if 0
void acpi_create_gnvs(global_nvs_t *gnvs)
{
	gnvs->apic = 1;
	gnvs->mpen = 1; /* Enable Multi Processing */

	/* Enable both COM ports */
	gnvs->cmap = 0x01;
	gnvs->cmbp = 0x01;
}

void acpi_create_fadt(acpi_fadt_t *fadt, acpi_facs_t *facs, void *dsdt)
{
	acpi_header_t *header = &(fadt->header);
	u16 pmbase = 0;

	// TODO: find pmbase
	memset((void *) fadt, 0, sizeof(acpi_fadt_t));
	memcpy(header->signature, "FACP", 4);
	header->length = sizeof(acpi_fadt_t);
	header->revision = get_acpi_table_revision(FADT);
	memcpy(header->oem_id, OEM_ID, 6);
	memcpy(header->oem_table_id, ACPI_TABLE_CREATOR, 8);
	memcpy(header->asl_compiler_id, ASLC, 4);
	header->asl_compiler_revision = asl_revision;

	fadt->firmware_ctrl = (unsigned long) facs;
	fadt->dsdt = (unsigned long) dsdt;
	fadt->reserved = 0x00;
	fadt->preferred_pm_profile = PM_MOBILE;
	fadt->sci_int = 0x9;
	fadt->smi_cmd = 0;
	fadt->acpi_enable = 0;
	fadt->acpi_disable = 0;
	fadt->s4bios_req = 0x0;
	fadt->pstate_cnt = 0;

	fadt->pm1a_evt_blk = pmbase;
	fadt->pm1b_evt_blk = 0x0;
	fadt->pm1a_cnt_blk = pmbase + 0x4;
	fadt->pm1b_cnt_blk = 0x0;
	fadt->pm2_cnt_blk = pmbase + 0x50;
	fadt->pm_tmr_blk = pmbase + 0x8;
	fadt->gpe0_blk = pmbase + 0x20;
	fadt->gpe1_blk = 0;

	fadt->pm1_evt_len = 4;
	fadt->pm1_cnt_len = 2; /* Upper word is reserved and
				  Linux complains about 32 bit. */
	fadt->pm2_cnt_len = 1;
	fadt->pm_tmr_len = 4;
	fadt->gpe0_blk_len = 16;
	fadt->gpe1_blk_len = 0;
	fadt->gpe1_base = 0;
	fadt->cst_cnt = 0;
	fadt->p_lvl2_lat = 1;
	fadt->p_lvl3_lat = 0x39;
	fadt->flush_size = 0;
	fadt->flush_stride = 0;
	fadt->duty_offset = 1;
	fadt->duty_width = 3;
	fadt->day_alrm = 0xd;
	fadt->mon_alrm = 0x00;
	fadt->century = 0x32;
	fadt->iapc_boot_arch = ACPI_FADT_LEGACY_FREE;
	fadt->flags = ACPI_FADT_WBINVD | ACPI_FADT_C1_SUPPORTED |
			ACPI_FADT_SLEEP_BUTTON | ACPI_FADT_S4_RTC_WAKE |
			ACPI_FADT_DOCKING_SUPPORTED | ACPI_FADT_RESET_REGISTER |
			ACPI_FADT_PLATFORM_CLOCK;

	fadt->reset_reg.space_id = ACPI_ADDRESS_SPACE_IO;
	fadt->reset_reg.bit_width = 8;
	fadt->reset_reg.bit_offset = 0;
	fadt->reset_reg.access_size = ACPI_ACCESS_SIZE_BYTE_ACCESS;
	fadt->reset_reg.addrl = 0xcf9;
	fadt->reset_reg.addrh = 0;
	fadt->reset_value = 0x06;

	fadt->x_firmware_ctl_l = 0; /* Set X_FIRMWARE_CTRL only if FACS is */
	fadt->x_firmware_ctl_h = 0; /* above 4GB. If X_FIRMWARE_CTRL is set, */
				    /* then FIRMWARE_CTRL must be zero. */
	fadt->x_dsdt_l = (unsigned long)dsdt;
	fadt->x_dsdt_h = 0;

	fadt->x_pm1a_evt_blk.space_id = 1;
	fadt->x_pm1a_evt_blk.bit_width = 32;
	fadt->x_pm1a_evt_blk.bit_offset = 0;
	fadt->x_pm1a_evt_blk.access_size = ACPI_ACCESS_SIZE_DWORD_ACCESS;
	fadt->x_pm1a_evt_blk.addrl = pmbase;
	fadt->x_pm1a_evt_blk.addrh = 0x0;

	fadt->x_pm1b_evt_blk.space_id = 0;
	fadt->x_pm1b_evt_blk.bit_width = 0;
	fadt->x_pm1b_evt_blk.bit_offset = 0;
	fadt->x_pm1b_evt_blk.access_size = 0;
	fadt->x_pm1b_evt_blk.addrl = 0x0;
	fadt->x_pm1b_evt_blk.addrh = 0x0;

	fadt->x_pm1a_cnt_blk.space_id = 1;
	fadt->x_pm1a_cnt_blk.bit_width = 16; /* Upper word is reserved and
						Linux complains about 32 bit. */
	fadt->x_pm1a_cnt_blk.bit_offset = 0;
	fadt->x_pm1a_cnt_blk.access_size = ACPI_ACCESS_SIZE_WORD_ACCESS;
	fadt->x_pm1a_cnt_blk.addrl = pmbase + 0x4;
	fadt->x_pm1a_cnt_blk.addrh = 0x0;

	fadt->x_pm1b_cnt_blk.space_id = 0;
	fadt->x_pm1b_cnt_blk.bit_width = 0;
	fadt->x_pm1b_cnt_blk.bit_offset = 0;
	fadt->x_pm1b_cnt_blk.access_size = 0;
	fadt->x_pm1b_cnt_blk.addrl = 0x0;
	fadt->x_pm1b_cnt_blk.addrh = 0x0;

	fadt->x_pm2_cnt_blk.space_id = 1;
	fadt->x_pm2_cnt_blk.bit_width = 8;
	fadt->x_pm2_cnt_blk.bit_offset = 0;
	fadt->x_pm2_cnt_blk.access_size = ACPI_ACCESS_SIZE_BYTE_ACCESS;
	fadt->x_pm2_cnt_blk.addrl = pmbase + 0x50;
	fadt->x_pm2_cnt_blk.addrh = 0x0;

	fadt->x_pm_tmr_blk.space_id = 1;
	fadt->x_pm_tmr_blk.bit_width = 32;
	fadt->x_pm_tmr_blk.bit_offset = 0;
	fadt->x_pm_tmr_blk.access_size = ACPI_ACCESS_SIZE_DWORD_ACCESS;
	fadt->x_pm_tmr_blk.addrl = pmbase + 0x8;
	fadt->x_pm_tmr_blk.addrh = 0x0;

	fadt->x_gpe0_blk.space_id = 1;
	fadt->x_gpe0_blk.bit_width = 128;
	fadt->x_gpe0_blk.bit_offset = 0;
	fadt->x_gpe0_blk.access_size = 0;
	fadt->x_gpe0_blk.addrl = pmbase + 0x20;
	fadt->x_gpe0_blk.addrh = 0x0;

	fadt->x_gpe1_blk.space_id = 0;
	fadt->x_gpe1_blk.bit_width = 0;
	fadt->x_gpe1_blk.bit_offset = 0;
	fadt->x_gpe1_blk.access_size = 0;
	fadt->x_gpe1_blk.addrl = 0x0;
	fadt->x_gpe1_blk.addrh = 0x0;

	header->checksum = acpi_checksum((void *) fadt, header->length);
}

unsigned long acpi_fill_madt(unsigned long current)
{
	/* Local APICs */
	current = acpi_create_madt_lapics(current);

	/* IOAPIC */
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *) current,
				2, IO_APIC_ADDR, 0);

	/* INT_SRC_OVR */
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)
		 current, 0, 0, 2, 0);
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)
		 current, 0, 9, 9, MP_IRQ_TRIGGER_LEVEL | MP_IRQ_POLARITY_HIGH);

	return current;
}

unsigned long acpi_fill_mcfg(unsigned long current)
{
	struct device *dev;
	u32 reg;

	dev = dev_find_device(0x8086, 0x29c0, 0);
	if (!dev)
		return current;

	reg = pci_read_config32(dev, 0x60);
	if ((reg & 0x07) != 0x01)  /* require enabled + 256MB size */
		return current;

	current += acpi_create_mcfg_mmconfig((acpi_mcfg_mmconfig_t *) current,
					     reg & 0xf0000000, 0x0, 0x0, 255);
	return current;
}
#endif

// bogus
#if 0
unsigned long acpi_fill_madt(unsigned long current)
{
	/* create all subtables for processors */
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x0, 0x0);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x2, 0x2);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x4, 0x4);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x6, 0x6);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x8, 0x8);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa, 0xa);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc, 0xc);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe, 0xe);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x10, 0x10);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x12, 0x12);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x14, 0x14);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x16, 0x16);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x18, 0x18);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x1a, 0x1a);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x1c, 0x1c);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x1e, 0x1e);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x20, 0x20);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x22, 0x22);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x24, 0x24);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x26, 0x26);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x28, 0x28);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x2a, 0x2a);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x2c, 0x2c);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x2e, 0x2e);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x30, 0x30);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x32, 0x32);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x34, 0x34);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x36, 0x36);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x38, 0x38);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x3a, 0x3a);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x3c, 0x3c);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x3e, 0x3e);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x40, 0x80);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x42, 0x82);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x44, 0x84);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x46, 0x86);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x48, 0x88);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x4a, 0x8a);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x4c, 0x8c);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x4e, 0x8e);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x50, 0x90);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x52, 0x92);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x54, 0x94);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x56, 0x96);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x58, 0x98);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x5a, 0x9a);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x5c, 0x9c);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x5e, 0x9e);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x60, 0xa0);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x62, 0xa2);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x64, 0xa4);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x66, 0xa6);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x68, 0xa8);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x6a, 0xaa);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x6c, 0xac);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x6e, 0xae);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x70, 0xb0);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x72, 0xb2);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x74, 0xb4);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x76, 0xb6);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x78, 0xb8);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x7a, 0xba);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x7c, 0xbc);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x7e, 0xbe);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x1, 0x1);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x3, 0x3);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x5, 0x5);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x7, 0x7);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x9, 0x9);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb, 0xb);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd, 0xd);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf, 0xf);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x11, 0x11);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x13, 0x13);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x15, 0x15);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x17, 0x17);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x19, 0x19);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x1b, 0x1b);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x1d, 0x1d);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x1f, 0x1f);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x21, 0x21);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x23, 0x23);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x25, 0x25);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x27, 0x27);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x29, 0x29);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x2b, 0x2b);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x2d, 0x2d);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x2f, 0x2f);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x31, 0x31);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x33, 0x33);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x35, 0x35);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x37, 0x37);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x39, 0x39);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x3b, 0x3b);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x3d, 0x3d);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x3f, 0x3f);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x41, 0x81);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x43, 0x83);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x45, 0x85);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x47, 0x87);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x49, 0x89);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x4b, 0x8b);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x4d, 0x8d);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x4f, 0x8f);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x51, 0x91);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x53, 0x93);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x55, 0x95);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x57, 0x97);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x59, 0x99);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x5b, 0x9b);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x5d, 0x9d);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x5f, 0x9f);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x61, 0xa1);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x63, 0xa3);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x65, 0xa5);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x67, 0xa7);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x69, 0xa9);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x6b, 0xab);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x6d, 0xad);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x6f, 0xaf);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x71, 0xb1);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x73, 0xb3);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x75, 0xb5);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x77, 0xb7);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x79, 0xb9);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x7b, 0xbb);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x7d, 0xbd);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x7f, 0xbf);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x80, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x81, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x82, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x83, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x84, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x85, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x86, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x87, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x88, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x89, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x8a, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x8b, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x8c, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x8d, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x8e, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x8f, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x90, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x91, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x92, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x93, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x94, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x95, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x96, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x97, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x98, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x99, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x9a, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x9b, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x9c, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x9d, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x9e, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0x9f, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa0, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa1, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa2, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa3, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa4, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa5, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa6, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa7, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa8, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xa9, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xaa, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xab, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xac, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xad, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xae, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xaf, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb0, 0xff);
	c        current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current, 0xf8, 0xb4280000, 0x000000f8);
        current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)current, 0x0, 0x0, 0x00000002, 0x0000);
        current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)current, 0x0, 0x9, 0x00000009, 0x000f);urrent += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb1, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb2, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb3, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb4, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb5, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb6, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb7, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb8, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xb9, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xba, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xbb, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xbc, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xbd, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xbe, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xbf, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc0, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc1, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc2, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc3, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc4, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc5, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc6, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc7, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc8, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xc9, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xca, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xcb, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xcc, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xcd, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xce, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xcf, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd0, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd1, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd2, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd3, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd4, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd5, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd6, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd7, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd8, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xd9, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xda, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xdb, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xdc, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xdd, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xde, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xdf, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe0, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe1, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe2, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe3, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe4, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe5, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe6, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe7, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe8, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xe9, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xea, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xeb, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xec, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xed, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xee, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xef, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf0, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf1, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf2, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf3, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf4, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf5, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf6, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf7, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf8, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xf9, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xfa, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xfb, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xfc, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xfd, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xfe, 0xff);
	current += acpi_create_madt_lapic((acpi_madt_lapic_t *)current, 0xff, 0xff);
	current += acpi_create_madt_lapic_nmi((acpi_madt_lapic_nmi_t *)current, 0xff, 0x0005, 0x1);
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current, 0xf0, 0xfec00000, 0x00000000);
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current, 0xf1, 0xc9280000, 0x00000018);
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current, 0xf2, 0xf4180000, 0x00000038);
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current, 0xf3, 0xc8180000, 0x00000058);
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current, 0xf4, 0xf5180000, 0x00000078);
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current, 0xf5, 0xbf280000, 0x00000098);
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current, 0xf6, 0xbe180000, 0x000000b8);
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current, 0xf7, 0xb5180000, 0x000000d8);
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current, 0xf8, 0xb4280000, 0x000000f8);
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)current, 0x0, 0x0, 0x00000002, 0x0000);
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)current, 0x0, 0x9, 0x00000009, 0x000f);
	return current;
}
#endif
