/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2003-2004 Eric Biederman
 * Copyright (C) 2005-2010 coresystems GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <arch/cbconfig.h>
#include <compiler.h>
#include <console/console.h>
#include <console/uart.h>
#include <ip_checksum.h>
#include <boot/coreboot_tables.h>
#include <boot/tables.h>
#include <boot_device.h>
#include <string.h>
#include <version.h>
#include <boardid.h>
#include <device/device.h>
#include <fmap.h>
#include <stdlib.h>
#include <cbfs.h>
#include <cbmem.h>
#include <bootmem.h>
#include <spi_flash.h>
#include <security/vboot/vbnv_layout.h>
#if IS_ENABLED(CONFIG_USE_OPTION_TABLE)
#include <option_table.h>
#endif
#if IS_ENABLED(CONFIG_CHROMEOS)
#if IS_ENABLED(CONFIG_HAVE_ACPI_TABLES)
#include <arch/acpi.h>
#endif
#include <vendorcode/google/chromeos/chromeos.h>
#include <vendorcode/google/chromeos/gnvs.h>
#endif
#if IS_ENABLED(CONFIG_ARCH_X86)
#include <cpu/x86/mtrr.h>
#endif
#include <commonlib/helpers.h>

static struct lb_header *lb_table_init(unsigned long addr)
{
	struct lb_header *header;

	/* 16 byte align the address */
	addr += 15;
	addr &= ~15;

	header = (void *)addr;
	header->signature[0] = 'L';
	header->signature[1] = 'B';
	header->signature[2] = 'I';
	header->signature[3] = 'O';
	header->header_bytes = sizeof(*header);
	header->header_checksum = 0;
	header->table_bytes = 0;
	header->table_checksum = 0;
	header->table_entries = 0;
	return header;
}

static struct lb_record *lb_first_record(struct lb_header *header)
{
	struct lb_record *rec;
	rec = (void *)(((char *)header) + sizeof(*header));
	return rec;
}

static struct lb_record *lb_last_record(struct lb_header *header)
{
	struct lb_record *rec;
	rec = (void *)(((char *)header) + sizeof(*header)
		+ header->table_bytes);
	return rec;
}

struct lb_record *lb_new_record(struct lb_header *header)
{
	struct lb_record *rec;
	rec = lb_last_record(header);
	if (header->table_entries)
		header->table_bytes += rec->size;
	rec = lb_last_record(header);
	header->table_entries++;
	rec->tag = LB_TAG_UNUSED;
	rec->size = sizeof(*rec);
	return rec;
}

static struct lb_memory *lb_memory(struct lb_header *header)
{
	struct lb_record *rec;
	struct lb_memory *mem;
	rec = lb_new_record(header);
	mem = (struct lb_memory *)rec;
	mem->tag = LB_TAG_MEMORY;
	mem->size = sizeof(*mem);
	return mem;
}

static unsigned long lb_table_fini(struct lb_header *head)
{
	struct lb_record *rec, *first_rec;
	rec = lb_last_record(head);
	if (head->table_entries)
		head->table_bytes += rec->size;

	first_rec = lb_first_record(head);
	head->table_checksum = compute_ip_checksum(first_rec,
		head->table_bytes);
	head->header_checksum = 0;
	head->header_checksum = compute_ip_checksum(head, sizeof(*head));
	printk(BIOS_DEBUG,
	       "Wrote coreboot table at: %p, 0x%x bytes, checksum %x\n",
	       head, head->table_bytes, head->table_checksum);
	return (unsigned long)rec + rec->size;
}

struct arange_entry {
	uint64_t begin;
	uint64_t end;
	unsigned long tag;
};

static const struct arange_entry bootmem[] = {
	{0, 0x4000000, IORESOURCE_CACHEABLE },
};

static void abootmem_write_memory_table(struct lb_memory *mem)
{
	int i;
	const struct arange_entry *r;
	struct lb_memory_range *lb_r;

	lb_r = &mem->map[0];

	//bootmem_init();
	//bootmem_dump_ranges();

	for (i = 0; i < 1; i++) {
		r = &bootmem[i];
		lb_r->start = pack_lb64(r->begin);
		lb_r->size = pack_lb64(r->end);
		lb_r->type = r->tag;
		lb_r++;
		mem->size += sizeof(struct lb_memory_range);
	}
}


static uintptr_t write_coreboot_table(uintptr_t rom_table_end)
{
	struct lb_header *head;
	unsigned long base = 32;

	printk(BIOS_DEBUG, "Writing coreboot table at 0x%08lx\n", base);


	head = lb_table_init(base);

#if IS_ENABLED(CONFIG_USE_OPTION_TABLE)
	{
		struct cmos_option_table *option_table =
			cbfs_boot_map_with_leak("cmos_layout.bin",
				CBFS_COMPONENT_CMOS_LAYOUT, NULL);
		if (option_table) {
			struct lb_record *rec_dest = lb_new_record(head);
			/* Copy the option config table, it's already a
			 * lb_record...
			 */
			memcpy(rec_dest,  option_table, option_table->size);
			/* Create cmos checksum entry in coreboot table */
			lb_cmos_checksum(head);
		} else {
			printk(BIOS_ERR,
				"cmos_layout.bin could not be found!\n");
		}
	}
#endif

	/* Serialize resource map into mem table types (LB_MEM_*) */
	abootmem_write_memory_table(lb_memory(head));

	/* Remember where my valid memory ranges are */
	return lb_table_fini(head);
}

void write_tables(void)
{
	uintptr_t cbtable_start;
	uintptr_t cbtable_end;
	size_t cbtable_size;
	const size_t max_table_size = COREBOOT_TABLE_SIZE;

	//(uintptr_t)cbmem_add(CBMEM_ID_CBTABLE, max_table_size);
	cbtable_start = 0x20;

	if (!cbtable_start) {
		printk(BIOS_ERR, "Could not add CBMEM for coreboot table.\n");
		return;
	}

	/* Add architecture specific tables. */
	//arch_write_tables(cbtable_start);

	/* Write the coreboot table. */
	cbtable_end = write_coreboot_table(cbtable_start);
	cbtable_size = cbtable_end - cbtable_start;

	if (cbtable_size > max_table_size) {
		printk(BIOS_ERR, "%s: coreboot table didn't fit (%zx/%zx)\n",
			__func__, cbtable_size, max_table_size);
	}

	printk(BIOS_DEBUG, "coreboot table: %zd bytes.\n", cbtable_size);

	/* Print CBMEM sections */
	//cbmem_list();
}
