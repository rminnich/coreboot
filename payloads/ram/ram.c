/*
 * This file is part of the coreinfo project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
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

#include "ram.h"

int main(void)
{
	printf("Morning\n");
	struct cbfs_file *f;

#if IS_ENABLED(CONFIG_RAMPAYLOAD_ACPI)
	// ACPI table loading goes here
#endif

	f = cbfs_find("fallback/linux");
	if (!f)
		die("cbfs_find failed");

	die("FOUND");
	return 0;
}

PAYLOAD_INFO(name, CONFIG_PAYLOAD_INFO_NAME);
PAYLOAD_INFO(listname, CONFIG_PAYLOAD_INFO_LISTNAME);
PAYLOAD_INFO(desc, CONFIG_PAYLOAD_INFO_DESC);
