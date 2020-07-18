/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <cbmem.h>

void *cbmem_top_chipset(void)
{
	return (void *)(2048*1024*1024ULL);
}
