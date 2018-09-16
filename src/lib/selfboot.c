/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2003 Eric W. Biederman <ebiederm@xmission.com>
 * Copyright (C) 2009 Ron Minnich <rminnich@gmail.com>
 * Copyright (C) 2016 George Trudeau <george.trudeau@usherbrooke.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <commonlib/compression.h>
#include <commonlib/endian.h>
#include <console/console.h>
#include <cpu/cpu.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <symbols.h>
#include <cbfs.h>
#include <lib.h>
#include <bootmem.h>
#include <program_loading.h>
#include <timestamp.h>
#include <cbmem.h>

struct segment {
	unsigned long s_dstaddr;
	unsigned long s_srcaddr;
	unsigned long s_memsz;
	unsigned long s_filesz;
	int final;
	int compression;
};

/* Decode a serialized cbfs payload segment
 * from memory into native endianness.
 */
static void cbfs_decode_payload_segment(struct cbfs_payload_segment *segment,
		const struct cbfs_payload_segment *src)
{
	segment->type        = read_be32(&src->type);
	segment->compression = read_be32(&src->compression);
	segment->offset      = read_be32(&src->offset);
	segment->load_addr   = read_be64(&src->load_addr);
	segment->len         = read_be32(&src->len);
	segment->mem_len     = read_be32(&src->mem_len);
}

static int segment_targets_usable_ram(struct segment *ptr)
{
	if (bootmem_region_targets_usable_ram(ptr->s_dstaddr, ptr->s_memsz))
		return 1;
	
	if (payload_arch_usable_ram_quirk(ptr->s_dstaddr, ptr->s_memsz))
		return 1;
	
	/* Payload segment not targeting RAM. */
	printk(BIOS_ERR, "SELF Payload doesn't target RAM:\n");
	printk(BIOS_ERR, "Failed Segment: 0x%lx, %lu bytes\n", ptr->s_dstaddr, ptr->s_memsz);
	bootmem_dump_ranges();
	return 0;
}

static int load_one_segment(struct segment *ptr)
{
		unsigned char *dest, *src, *middle, *end;
		size_t len, memsz;
		printk(BIOS_DEBUG,
			"Loading Segment: addr: 0x%016lx memsz: 0x%016lx filesz: 0x%016lx\n",
			ptr->s_dstaddr, ptr->s_memsz, ptr->s_filesz);

		/* Compute the boundaries of the segment */
		dest = (unsigned char *)(ptr->s_dstaddr);
		src = (unsigned char *)(ptr->s_srcaddr);
		len = ptr->s_filesz;
		memsz = ptr->s_memsz;
		end = dest + memsz;

		/* Copy data from the initial buffer */
		switch (ptr->compression) {
		case CBFS_COMPRESS_LZMA: {
			printk(BIOS_DEBUG, "using LZMA\n");
			timestamp_add_now(TS_START_ULZMA);
			len = ulzman(src, len, dest, memsz);
			timestamp_add_now(TS_END_ULZMA);
			if (!len) /* Decompression Error. */
				return 0;
			break;
		}
		case CBFS_COMPRESS_LZ4: {
			printk(BIOS_DEBUG, "using LZ4\n");
			timestamp_add_now(TS_START_ULZ4F);
			len = ulz4fn(src, len, dest, memsz);
			timestamp_add_now(TS_END_ULZ4F);
			if (!len) /* Decompression Error. */
				return 0;
			break;
		}
		case CBFS_COMPRESS_NONE: {
			printk(BIOS_DEBUG, "it's not compressed!\n");
			memcpy(dest, src, len);
			break;
		}
		default:
			printk(BIOS_INFO,  "CBFS:  Unknown compression type %d\n",
				ptr->compression);
			return -1;
		}
		/* Calculate middle after any changes to len. */
		middle = dest + len;
		printk(BIOS_SPEW, "[ 0x%08lx, %08lx, 0x%08lx) <- %08lx\n",
			(unsigned long)dest,
			(unsigned long)middle,
			(unsigned long)end,
			(unsigned long)src);

		/* Zero the extra bytes between middle & end */
		if (middle < end) {
			printk(BIOS_DEBUG,
				"Clearing Segment: addr: 0x%016lx memsz: 0x%016lx\n",
				(unsigned long)middle,
				(unsigned long)(end - middle));

			/* Zero the extra bytes */
			memset(middle, 0, end - middle);
		}

		/*
		 * Each architecture can perform additional operations
		 * on the loaded segment
		 */
		prog_segment_loaded((uintptr_t)dest, ptr->s_memsz, ptr->final);


	return 1;
}

static int load_payload_segments(
	struct cbfs_payload_segment *cbfssegs,
	int check_regions,
	uintptr_t *entry)
{
	struct segment new;
	struct cbfs_payload_segment *seg, segment, *first_segment = cbfssegs;

	for (seg = cbfssegs;; ++seg) {
		memset(&new, 0, sizeof(new));
		printk(BIOS_DEBUG,
			"Loading segment from ROM address 0x%p\n",
			seg);

		cbfs_decode_payload_segment(&segment, seg);

		switch (segment.type) {
		case PAYLOAD_SEGMENT_CODE:
		case PAYLOAD_SEGMENT_DATA:
			printk(BIOS_DEBUG, "  %s (compression=%x)\n",
				segment.type == PAYLOAD_SEGMENT_CODE
				?  "code" : "data", segment.compression);

			new.s_dstaddr = segment.load_addr;
			new.s_memsz = segment.mem_len;
			new.compression = segment.compression;
			new.s_srcaddr = (uintptr_t)
				((unsigned char *)first_segment)
				+ segment.offset;
			new.s_filesz = segment.len;

			printk(BIOS_DEBUG,
				"  New segment dstaddr 0x%lx memsize 0x%lx srcaddr 0x%lx filesize 0x%lx\n",
				new.s_dstaddr, new.s_memsz, new.s_srcaddr,
				new.s_filesz);

			/* Clean up the values */
			if (new.s_filesz > new.s_memsz)  {
				new.s_filesz = new.s_memsz;
				printk(BIOS_DEBUG,
					"  cleaned up filesize 0x%lx\n",
					new.s_filesz);
			}
			break;

		case PAYLOAD_SEGMENT_BSS:
			printk(BIOS_DEBUG, "  BSS 0x%p (%d byte)\n", (void *)
				(intptr_t)segment.load_addr, segment.mem_len);

			new.s_filesz = 0;
			new.s_srcaddr = (uintptr_t)
				((unsigned char *)first_segment)
				+ segment.offset;
			new.s_dstaddr = segment.load_addr;
			new.s_memsz = segment.mem_len;
			new.compression = CBFS_COMPRESS_NONE;
			break;

		case PAYLOAD_SEGMENT_ENTRY:
			printk(BIOS_DEBUG, "  Entry Point 0x%p\n", (void *)
				(intptr_t)segment.load_addr);

			*entry =  segment.load_addr;
			/* Per definition, a payload always has the entry point
			 * as last segment. Thus, we use the occurrence of the
			 * entry point as break condition for the loop.
			 * Can we actually just look at the number of section?
			 */
			return 0;
			break;

		default:
			/* We found something that we don't know about. Throw
			 * hands into the sky and run away!
			 */
			printk(BIOS_EMERG, "Bad segment type %x\n",
				segment.type);
			return -1;
		}
		if (check_regions && !segment_targets_usable_ram(&new))
			return -1;
		if (load_one_segment(&new) < 1)
			return -1;
	}

	return 1;
}

__weak int payload_arch_usable_ram_quirk(uint64_t start, uint64_t size)
{
	return 0;
}

bool selfload(struct prog *payload, bool check_regions)
{
	uintptr_t entry = 0;
	void *data;

	data = rdev_mmap_full(prog_rdev(payload));

	if (data == NULL)
		return false;

	if (load_payload_segments(&((struct cbfs_payload *)data)->segments, check_regions, &entry))
		goto out;

	printk(BIOS_SPEW, "Loaded segments\n");

	rdev_munmap(prog_rdev(payload), data);

	/* Pass cbtables to payload if architecture desires it. */
	prog_set_entry(payload, (void *)entry, cbmem_find(CBMEM_ID_CBTABLE));

	return true;
out:
	rdev_munmap(prog_rdev(payload), data);
	return false;
}
