/* SPDX-License-Identifier: GPL-2.0-only */

#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include <smbios.h>
#include <console/console.h>
#include <arch/io.h>
#include <acpi/acpi.h>
#include <commonlib/endian.h>

const char *smbios_mainboard_manufacturer(void)
{
	return  CONFIG_MAINBOARD_SMBIOS_MANUFACTURER;
}

const char *smbios_mainboard_product_name(void)
{
	return  CONFIG_MAINBOARD_SMBIOS_PRODUCT_NAME;
}

const char *smbios_mainboard_version(void)
{
	return  CONFIG_MAINBOARD_VERSION;
}

const char *smbios_mainboard_serial_number(void)
{
	return  CONFIG_MAINBOARD_SERIAL_NUMBER;
}

void smbios_system_set_uuid(u8 *uuid)
{
	memset(uuid, 0, 16);
}

