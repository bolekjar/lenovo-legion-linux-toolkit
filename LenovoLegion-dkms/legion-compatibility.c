// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "legion-compatibility.h"

#include <linux/dmi.h>

const char* ALLOWED_VENDOR = "LENOVO";

const char* AllowedModelsPrefix[] = {
	// Worldwide variants
	"17ACH",
	"17ARH",
	"17ITH",
	"17IMH",

	"16ACH",
	"16AHP",
	"16APH",
	"16ARH",
	"16ARP",
	"16ARX",
	"16IAH",
	"16IAX",
	"16IRH",
	"16IRX",
	"16ITH",

	"15ACH",
	"15AHP",
	"15APH",
	"15ARH",
	"15ARP",
	"15IAH",
	"15IAX",
	"15IHU",
	"15IMH",
	"15IRH",
	"15ITH",

	"14APH",
	"14IRP",

	// Chinese variants
	"G5000",
	"R9000",
	"R7000",
	"Y9000",
	"Y7000",

	// Limited compatibility
	"17IR",
	"15IR",
	"15IC",
	"15IK"
};

static int is_dmi_compatible(struct dmi_machine_information* dmi_machine_info) {

	if(strcmp(dmi_machine_info->bios_vendor,ALLOWED_VENDOR) != 0)
	{
		return 0;
	}

	for (size_t i = 0; i < (sizeof(AllowedModelsPrefix)/sizeof(AllowedModelsPrefix[0])); i++)
	{
		if(strstr(dmi_machine_info->product_family,AllowedModelsPrefix[i]) != NULL)
		{
			return 1;
		}
	}

	return 0;
}

int is_compatible(struct dmi_machine_information* dmi_machine_info) {
	return is_dmi_compatible(dmi_machine_info);
}
