/* SPDX-License-Identifier: GPL-2.0-only */

#include <assert.h>
#include <cbmem.h>
#include <console/console.h>
#include <imd.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

/* For more details on implementation and usage please see the imd.h header. */

static const uint32_t IMD_ROOT_PTR_MAGIC = 0xc0389481;
static const uint32_t IMD_ENTRY_MAGIC = ~0xc0389481;
static const uint32_t SMALL_REGION_ID = CBMEM_ID_IMD_SMALL;
static const size_t LIMIT_ALIGN = 4096;

/* In-memory data structures. */
struct imd_root_pointer {
	uint32_t magic;
	/* Relative to upper limit/offset. */
	int32_t root_offset;
} __packed;

struct imd_entry {
	uint32_t magic;
	/* start is located relative to imd_root */
	int32_t start_offset;
	uint32_t size;
	uint32_t id;
} __packed;

struct imd_root {
	uint32_t max_entries;
	uint32_t num_entries;
	uint32_t flags;
	uint32_t entry_align;
	/* Used for fixing the size of an imd. Relative to the root. */
	int32_t max_offset;
	struct imd_entry entries[0];
} __packed;

#define IMD_FLAG_LOCKED 1

static void *relative_pointer(void *base, ssize_t offset)
{
	print_func_entry();
	intptr_t b = (intptr_t)base;
	b += offset;
	print_func_exit();
	return (void *)b;
}

static bool imd_root_pointer_valid(const struct imd_root_pointer *rp)
{
	print_func_entry();
	print_func_exit();
	return !!(rp->magic == IMD_ROOT_PTR_MAGIC);
}

static struct imd_root *imdr_root(const struct imdr *imdr)
{
	print_func_entry();
	print_func_exit();
	return imdr->r;
}

/*
 * The root pointer is relative to the upper limit of the imd. i.e. It sits
 * just below the upper limit.
 */
static struct imd_root_pointer *imdr_get_root_pointer(const struct imdr *imdr)
{
	print_func_entry();
	struct imd_root_pointer *rp;

	rp = relative_pointer((void *)imdr->limit, -sizeof(*rp));

	print_func_exit();
	return rp;
}

static void imd_link_root(struct imd_root_pointer *rp, struct imd_root *r)
{
	print_func_entry();
	rp->magic = IMD_ROOT_PTR_MAGIC;
	rp->root_offset = (int32_t)((intptr_t)r - (intptr_t)rp);
	print_func_exit();
}

static struct imd_entry *root_last_entry(struct imd_root *r)
{
	print_func_entry();
	print_func_exit();
	return &r->entries[r->num_entries - 1];
}

static size_t root_num_entries(size_t root_size)
{
	print_func_entry();
	size_t entries_size;

	entries_size = root_size;
	entries_size -= sizeof(struct imd_root_pointer);
	entries_size -= sizeof(struct imd_root);

	print_func_exit();
	return entries_size / sizeof(struct imd_entry);
}

static size_t imd_root_data_left(struct imd_root *r)
{
	print_func_entry();
	struct imd_entry *last_entry;

	last_entry = root_last_entry(r);

	if (r->max_offset != 0) {
		print_func_exit();
		return last_entry->start_offset - r->max_offset;
	}

	print_func_exit();
	return ~(size_t)0;
}

static bool root_is_locked(const struct imd_root *r)
{
	print_func_entry();
	print_func_exit();
	return !!(r->flags & IMD_FLAG_LOCKED);
}

static void imd_entry_assign(struct imd_entry *e, uint32_t id,
				ssize_t offset, size_t size)
{
	print_func_entry();
	e->magic = IMD_ENTRY_MAGIC;
	e->start_offset = offset;
	e->size = size;
	e->id = id;
	print_func_exit();
}

static void imdr_init(struct imdr *ir, void *upper_limit)
{
	print_func_entry();
	uintptr_t limit = (uintptr_t)upper_limit;
	/* Upper limit is aligned down to 4KiB */
	ir->limit = ALIGN_DOWN(limit, LIMIT_ALIGN);
	ir->r = NULL;
	print_func_exit();
}

static int imdr_create_empty(struct imdr *imdr, size_t root_size,
				size_t entry_align)
{
	print_func_entry();
	struct imd_root_pointer *rp;
	struct imd_root *r;
	struct imd_entry *e;
	ssize_t root_offset;

	if (!imdr->limit) {
		print_func_exit();
		return -1;
	}

	/* root_size and entry_align should be a power of 2. */
	assert(IS_POWER_OF_2(root_size));
	assert(IS_POWER_OF_2(entry_align));

	if (!imdr->limit) {
		print_func_exit();
		return -1;
	}

	/*
	 * root_size needs to be large enough to accommodate root pointer and
	 * root book keeping structure. The caller needs to ensure there's
	 * enough room for tracking individual allocations.
	 */
	if (root_size < (sizeof(*rp) + sizeof(*r))) {
		print_func_exit();
		return -1;
	}

	/* For simplicity don't allow sizes or alignments to exceed LIMIT_ALIGN.
	 */
	if (root_size > LIMIT_ALIGN || entry_align > LIMIT_ALIGN) {
		print_func_exit();
		return -1;
	}

	/* Additionally, don't handle an entry alignment > root_size. */
	if (entry_align > root_size) {
		print_func_exit();
		return -1;
	}

	rp = imdr_get_root_pointer(imdr);

	root_offset = -(ssize_t)root_size;
	/* Set root pointer. */
	imdr->r = relative_pointer((void *)imdr->limit, root_offset);
	r = imdr_root(imdr);
	imd_link_root(rp, r);

	memset(r, 0, sizeof(*r));
	r->entry_align = entry_align;

	/* Calculate size left for entries. */
	r->max_entries = root_num_entries(root_size);

	/* Fill in first entry covering the root region. */
	r->num_entries = 1;
	e = &r->entries[0];
	imd_entry_assign(e, CBMEM_ID_IMD_ROOT, 0, root_size);

	printk(BIOS_DEBUG, "IMD: root @ %p %u entries.\n", r, r->max_entries);

	print_func_exit();
	return 0;
}

static int imdr_recover(struct imdr *imdr)
{
	print_func_entry();
	struct imd_root_pointer *rp;
	struct imd_root *r;
	uintptr_t low_limit;
	size_t i;

	if (!imdr->limit) {
		print_func_exit();
		return -1;
	}

	rp = imdr_get_root_pointer(imdr);

	if (!imd_root_pointer_valid(rp)) {
		print_func_exit();
		return -1;
	}

	r = relative_pointer(rp, rp->root_offset);

	/* Confirm the root and root pointer are just under the limit. */
	if (ALIGN_UP((uintptr_t)&r->entries[r->max_entries], LIMIT_ALIGN) !=
			imdr->limit) {
		print_func_exit();
		return -1;
	}

	if (r->num_entries > r->max_entries) {
		print_func_exit();
		return -1;
	}

	/* Entry alignment should be power of 2. */
	if (!IS_POWER_OF_2(r->entry_align)) {
		print_func_exit();
		return -1;
	}

	low_limit = (uintptr_t)relative_pointer(r, r->max_offset);

	/* If no max_offset then lowest limit is 0. */
	if (low_limit == (uintptr_t)r)
		low_limit = 0;

	for (i = 0; i < r->num_entries; i++) {
		uintptr_t start_addr;
		const struct imd_entry *e = &r->entries[i];

		if (e->magic != IMD_ENTRY_MAGIC) {
			print_func_exit();
			return -1;
		}

		start_addr = (uintptr_t)relative_pointer(r, e->start_offset);
		if (start_addr  < low_limit) {
			print_func_exit();
			return -1;
		}
		if (start_addr >= imdr->limit ||
				(start_addr + e->size) > imdr->limit) {
			print_func_exit();
			return -1;
		}
	}

	/* Set root pointer. */
	imdr->r = r;

	print_func_exit();
	return 0;
}

static const struct imd_entry *imdr_entry_find(const struct imdr *imdr,
						uint32_t id)
{
	print_func_entry();
	struct imd_root *r;
	struct imd_entry *e;
	size_t i;

	r = imdr_root(imdr);

	if (r == NULL) {
		print_func_exit();
		return NULL;
	}

	e = NULL;
	/* Skip first entry covering the root. */
	for (i = 1; i < r->num_entries; i++) {
		if (id != r->entries[i].id)
			continue;
		e = &r->entries[i];
		break;
	}

	print_func_exit();
	return e;
}

static int imdr_limit_size(struct imdr *imdr, size_t max_size)
{
	print_func_entry();
	struct imd_root *r;
	ssize_t smax_size;
	size_t root_size;

	r = imdr_root(imdr);
	if (r == NULL) {
		print_func_exit();
		return -1;
	}

	root_size = imdr->limit - (uintptr_t)r;

	if (max_size < root_size) {
		print_func_exit();
		return -1;
	}

	/* Take into account the root size. */
	smax_size = max_size - root_size;
	smax_size = -smax_size;

	r->max_offset = smax_size;

	print_func_exit();
	return 0;
}

static size_t imdr_entry_size(const struct imdr *imdr,
				const struct imd_entry *e)
{
	print_func_entry();
	print_func_exit();
	return e->size;
}

static void *imdr_entry_at(const struct imdr *imdr, const struct imd_entry *e)
{
	print_func_entry();
	print_func_exit();
	return relative_pointer(imdr_root(imdr), e->start_offset);
}

static struct imd_entry *imd_entry_add_to_root(struct imd_root *r, uint32_t id,
						size_t size)
{
	print_func_entry();
	struct imd_entry *entry;
	struct imd_entry *last_entry;
	ssize_t e_offset;
	size_t used_size;

	if (r->num_entries == r->max_entries) {
		print_func_exit();
		return NULL;
	}

	/* Determine total size taken up by entry. */
	used_size = ALIGN_UP(size, r->entry_align);

	/* See if size overflows imd total size. */
	if (used_size > imd_root_data_left(r)) {
		print_func_exit();
		return NULL;
	}

	/*
	 * Determine if offset field overflows. All offsets should be lower
	 * than the previous one.
	 */
	last_entry = root_last_entry(r);
	e_offset = last_entry->start_offset;
	e_offset -= (ssize_t)used_size;
	if (e_offset > last_entry->start_offset) {
		print_func_exit();
		return NULL;
	}

	entry = root_last_entry(r) + 1;
	r->num_entries++;

	imd_entry_assign(entry, id, e_offset, size);

	print_func_exit();
	return entry;
}

static const struct imd_entry *imdr_entry_add(const struct imdr *imdr,
						uint32_t id, size_t size)
{
	print_func_entry();
	struct imd_root *r;

	r = imdr_root(imdr);

	if (r == NULL) {
		print_func_exit();
		return NULL;
	}

	if (root_is_locked(r)) {
		print_func_exit();
		return NULL;
	}

	print_func_exit();
	return imd_entry_add_to_root(r, id, size);
}

static bool imdr_has_entry(const struct imdr *imdr, const struct imd_entry *e)
{
	print_func_entry();
	struct imd_root *r;
	size_t idx;

	r = imdr_root(imdr);
	if (r == NULL) {
		print_func_exit();
		return false;
	}

	/* Determine if the entry is within this root structure. */
	idx = e - &r->entries[0];
	if (idx >= r->num_entries) {
		print_func_exit();
		return false;
	}

	print_func_exit();
	return true;
}

static const struct imdr *imd_entry_to_imdr(const struct imd *imd,
						const struct imd_entry *entry)
{
	print_func_entry();
	if (imdr_has_entry(&imd->lg, entry)) {
		print_func_exit();
		return &imd->lg;
	}

	if (imdr_has_entry(&imd->sm, entry)) {
		print_func_exit();
		return &imd->sm;
	}

	print_func_exit();
	return NULL;
}

/* Initialize imd handle. */
void imd_handle_init(struct imd *imd, void *upper_limit)
{
	print_func_entry();
	imdr_init(&imd->lg, upper_limit);
	imdr_init(&imd->sm, NULL);
	print_func_exit();
}

void imd_handle_init_partial_recovery(struct imd *imd)
{
	print_func_entry();
	const struct imd_entry *e;
	struct imd_root_pointer *rp;
	struct imdr *imdr;

	if (imd->lg.limit == 0) {
		print_func_exit();
		return;
	}

	imd_handle_init(imd, (void *)imd->lg.limit);

	/* Initialize root pointer for the large regions. */
	imdr = &imd->lg;
	rp = imdr_get_root_pointer(imdr);
	imdr->r = relative_pointer(rp, rp->root_offset);

	e = imdr_entry_find(imdr, SMALL_REGION_ID);

	if (e == NULL) {
		print_func_exit();
		return;
	}

	imd->sm.limit = (uintptr_t)imdr_entry_at(imdr, e);
	imd->sm.limit += imdr_entry_size(imdr, e);
	imdr = &imd->sm;
	rp = imdr_get_root_pointer(imdr);
	imdr->r = relative_pointer(rp, rp->root_offset);
	print_func_exit();
}

int imd_create_empty(struct imd *imd, size_t root_size, size_t entry_align)
{
	print_func_entry();
	print_func_exit();
	return imdr_create_empty(&imd->lg, root_size, entry_align);
}

int imd_create_tiered_empty(struct imd *imd,
				size_t lg_root_size, size_t lg_entry_align,
				size_t sm_root_size, size_t sm_entry_align)
{
	print_func_entry();
	size_t sm_region_size;
	const struct imd_entry *e;
	struct imdr *imdr;

	imdr = &imd->lg;

	if (imdr_create_empty(imdr, lg_root_size, lg_entry_align) != 0) {
		print_func_exit();
		return -1;
	}

	/* Calculate the size of the small region to request. */
	sm_region_size = root_num_entries(sm_root_size) * sm_entry_align;
	sm_region_size += sm_root_size;
	sm_region_size = ALIGN_UP(sm_region_size, lg_entry_align);

	/* Add a new entry to the large region to cover the root and entries. */
	e = imdr_entry_add(imdr, SMALL_REGION_ID, sm_region_size);

	if (e == NULL)
		goto fail;

	imd->sm.limit = (uintptr_t)imdr_entry_at(imdr, e);
	imd->sm.limit += sm_region_size;

	if (imdr_create_empty(&imd->sm, sm_root_size, sm_entry_align) != 0 ||
		imdr_limit_size(&imd->sm, sm_region_size))
		goto fail;

	print_func_exit();
	return 0;
fail:
	imd_handle_init(imd, (void *)imdr->limit);
	print_func_exit();
	return -1;
}

int imd_recover(struct imd *imd)
{
	print_func_entry();
	const struct imd_entry *e;
	uintptr_t small_upper_limit;
	struct imdr *imdr;

	imdr = &imd->lg;
	if (imdr_recover(imdr) != 0) {
		print_func_exit();
		return -1;
	}

	/* Determine if small region is region is present. */
	e = imdr_entry_find(imdr, SMALL_REGION_ID);

	if (e == NULL) {
		print_func_exit();
		return 0;
	}

	small_upper_limit = (uintptr_t)imdr_entry_at(imdr, e);
	small_upper_limit += imdr_entry_size(imdr, e);

	imd->sm.limit = small_upper_limit;

	/* Tear down any changes on failure. */
	if (imdr_recover(&imd->sm) != 0) {
		imd_handle_init(imd, (void *)imd->lg.limit);
		print_func_exit();
		return -1;
	}

	print_func_exit();
	return 0;
}

int imd_limit_size(struct imd *imd, size_t max_size)
{
	print_func_entry();
	print_func_exit();
	return imdr_limit_size(&imd->lg, max_size);
}

int imd_lockdown(struct imd *imd)
{
	print_func_entry();
	struct imd_root *r;

	r = imdr_root(&imd->lg);
	if (r == NULL) {
		print_func_exit();
		return -1;
	}

	r->flags |= IMD_FLAG_LOCKED;

	r = imdr_root(&imd->sm);
	if (r != NULL)
		r->flags |= IMD_FLAG_LOCKED;

	print_func_exit();
	return 0;
}

int imd_region_used(struct imd *imd, void **base, size_t *size)
{
	print_func_entry();
	struct imd_root *r;
	struct imd_entry *e;
	void *low_addr;
	size_t sz_used;

	if (!imd->lg.limit) {
		print_func_exit();
		return -1;
	}

	r = imdr_root(&imd->lg);

	if (r == NULL) {
		print_func_exit();
		return -1;
	}

	/* Use last entry to obtain lowest address. */
	e = root_last_entry(r);

	low_addr = relative_pointer(r, e->start_offset);

	/* Total size used is the last entry's base up to the limit. */
	sz_used = imd->lg.limit - (uintptr_t)low_addr;

	*base = low_addr;
	*size = sz_used;

	print_func_exit();
	return 0;
}

const struct imd_entry *imd_entry_add(const struct imd *imd, uint32_t id,
					size_t size)
{
	print_func_entry();
	struct imd_root *r;
	const struct imdr *imdr;
	const struct imd_entry *e = NULL;

	/*
	 * Determine if requested size is less than 1/4 of small data
	 * region is left.
	 */
	imdr = &imd->sm;
	r = imdr_root(imdr);

	/* No small region. Use the large region. */
	if (r == NULL) {
		print_func_exit();
		return imdr_entry_add(&imd->lg, id, size);
	}
	else if (size <= r->entry_align || size <= imd_root_data_left(r) / 4)
		e = imdr_entry_add(imdr, id, size);

	/* Fall back on large region allocation. */
	if (e == NULL)
		e = imdr_entry_add(&imd->lg, id, size);

	print_func_exit();
	return e;
}

const struct imd_entry *imd_entry_find(const struct imd *imd, uint32_t id)
{
	print_func_entry();
	const struct imd_entry *e;

	/* Many of the smaller allocations are used a lot. Therefore, try
	 * the small region first. */
	e = imdr_entry_find(&imd->sm, id);

	if (e == NULL)
		e = imdr_entry_find(&imd->lg, id);

	print_func_exit();
	return e;
}

const struct imd_entry *imd_entry_find_or_add(const struct imd *imd,
						uint32_t id, size_t size)
{
	print_func_entry();
	const struct imd_entry *e;

	e = imd_entry_find(imd, id);

	if (e != NULL) {
		print_func_exit();
		return e;
	}

	print_func_exit();
	return imd_entry_add(imd, id, size);
}

size_t imd_entry_size(const struct imd *imd, const struct imd_entry *entry)
{
	print_func_entry();
	print_func_exit();
	return imdr_entry_size(NULL, entry);
}

void *imd_entry_at(const struct imd *imd, const struct imd_entry *entry)
{
	print_func_entry();
	const struct imdr *imdr;

	imdr = imd_entry_to_imdr(imd, entry);

	if (imdr == NULL) {
		print_func_exit();
		return NULL;
	}

	print_func_exit();
	return imdr_entry_at(imdr, entry);
}

uint32_t imd_entry_id(const struct imd *imd, const struct imd_entry *entry)
{
	print_func_entry();
	print_func_exit();
	return entry->id;
}

int imd_entry_remove(const struct imd *imd, const struct imd_entry *entry)
{
	print_func_entry();
	struct imd_root *r;
	const struct imdr *imdr;

	imdr = imd_entry_to_imdr(imd, entry);

	if (imdr == NULL) {
		print_func_exit();
		return -1;
	}

	r = imdr_root(imdr);

	if (r == NULL) {
		print_func_exit();
		return -1;
	}

	if (root_is_locked(r)) {
		print_func_exit();
		return -1;
	}

	if (entry != root_last_entry(r)) {
		print_func_exit();
		return -1;
	}

	r->num_entries--;

	print_func_exit();
	return 0;
}

static void imdr_print_entries(const struct imdr *imdr, const char *indent,
				const struct imd_lookup *lookup, size_t size)
{
	print_func_entry();
	struct imd_root *r;
	size_t i;
	size_t j;

	if (imdr == NULL) {
		print_func_exit();
		return;
	}

	r = imdr_root(imdr);

	for (i = 0; i < r->num_entries; i++) {
		const char *name = NULL;
		const struct imd_entry *e = &r->entries[i];

		for (j = 0; j < size; j++) {
			if (lookup[j].id == e->id) {
				name = lookup[j].name;
				break;
			}
		}

		printk(BIOS_DEBUG, "%s", indent);

		if (name == NULL)
			printk(BIOS_DEBUG, "%08x   ", e->id);
		else
			printk(BIOS_DEBUG, "%s", name);
		printk(BIOS_DEBUG, "%2zu. ", i);
		printk(BIOS_DEBUG, "%p ", imdr_entry_at(imdr, e));
		printk(BIOS_DEBUG, "0x%08zx\n", imdr_entry_size(imdr, e));
	}
	print_func_exit();
}

int imd_print_entries(const struct imd *imd, const struct imd_lookup *lookup,
			size_t size)
{
	print_func_entry();
	if (imdr_root(&imd->lg) == NULL) {
		print_func_exit();
		return -1;
	}

	imdr_print_entries(&imd->lg, "", lookup, size);
	if (imdr_root(&imd->sm) != NULL) {
		printk(BIOS_DEBUG, "IMD small region:\n");
		imdr_print_entries(&imd->sm, "  ", lookup, size);
	}

	print_func_exit();
	return 0;
}

int imd_cursor_init(const struct imd *imd, struct imd_cursor *cursor)
{
	print_func_entry();
	if (imd == NULL || cursor == NULL) {
		print_func_exit();
		return -1;
	}

	memset(cursor, 0, sizeof(*cursor));

	cursor->imdr[0] = &imd->lg;
	cursor->imdr[1] = &imd->sm;

	print_func_exit();
	return 0;
}

const struct imd_entry *imd_cursor_next(struct imd_cursor *cursor)
{
	print_func_entry();
	struct imd_root *r;
	const struct imd_entry *e;

	if (cursor->current_imdr >= ARRAY_SIZE(cursor->imdr)) {
		print_func_exit();
		return NULL;
	}

	r = imdr_root(cursor->imdr[cursor->current_imdr]);

	if (r == NULL) {
		print_func_exit();
		return NULL;
	}

	if (cursor->current_entry >= r->num_entries) {
		/* Try next imdr. */
		cursor->current_imdr++;
		cursor->current_entry = 0;
		print_func_exit();
		return imd_cursor_next(cursor);
	}

	e = &r->entries[cursor->current_entry];
	cursor->current_entry++;

	print_func_exit();
	return e;
}
