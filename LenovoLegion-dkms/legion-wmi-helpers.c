// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "legion-wmi-helpers.h"

#include <linux/acpi.h>
#include <linux/cleanup.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/wmi.h>


/**
 * legion_wmi_dev_evaluate_int() - Helper function for calling WMI methods that
 * return an integer.
 * @wdev: Pointer to the WMI device to be called.
 * @instance: Instance of the called method.
 * @method_id: WMI Method ID for the method to be called.
 * @buf: Buffer of all arguments for the given method_id.
 * @size: Length of the buffer.
 * @retval: Pointer for the return value to be assigned.
 *
 * Calls wmidev_evaluate_method for Lenovo WMI devices that return an ACPI
 * integer. Validates the return value type and assigns the value to the
 * retval pointer.
 *
 * Return: 0 on success, or an error code.
 */
int legion_wmi_dev_evaluate_int(struct wmi_device *wdev, u8 instance, u32 method_id,
			  unsigned char *buf, size_t size, u32 *retval)
{
	struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *ret_obj __free(kfree) = NULL;
	struct acpi_buffer input = { size, buf };
	acpi_status status;

	status = wmidev_evaluate_method(wdev, instance, method_id, &input,
					&output);
	if (ACPI_FAILURE(status)) {
		return -EIO;
	}

	ret_obj = output.pointer;

	if (retval) {
		if (!ret_obj) {
			return -ENODATA;
		}

		if (ret_obj->type != ACPI_TYPE_INTEGER)
			return -ENXIO;

		*retval = (u32)ret_obj->integer.value;
	}

	return 0;
};

/**
 * legion_wmi_dev_evaluate_string() - Helper function for calling WMI methods that
 * return an string.
 * @wdev: Pointer to the WMI device to be called.
 * @instance: Instance of the called method.
 * @method_id: WMI Method ID for the method to be called.
 * @buf: Buffer of all arguments for the given method_id.
 * @size: Length of the buffer.
 * @retval: Pointer for the return value to be assigned.
 * @retval_size: Length of the retval buffer.
 *
 * Calls wmidev_evaluate_method for Lenovo WMI devices that return an ACPI
 * integer. Validates the return value type and assigns the value to the
 * retval pointer.
 *
 * Return: 0 on success, or an error code.
 */
int legion_wmi_dev_evaluate_string(struct wmi_device *wdev, u8 instance, u32 method_id,
			  unsigned char *buf, size_t size, char *retval,size_t retval_size)
{
	struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *ret_obj __free(kfree) = NULL;
	struct acpi_buffer input = { size, buf };
	acpi_status status;

	status = wmidev_evaluate_method(wdev, instance, method_id, &input,
					&output);
	if (ACPI_FAILURE(status))
		return -EIO;

	ret_obj = output.pointer;

	if (retval) {
		if (!ret_obj)
			return -ENODATA;

		if (ret_obj->type != ACPI_TYPE_STRING)
			return -ENXIO;

		if(ret_obj->string.length >= retval_size)
		{
			return -EMSGSIZE;
		}

		strncpy(retval,ret_obj->string.pointer,ret_obj->string.length);
		retval[ret_obj->string.length] = '\0';  // Explicit NULL termination
	}

	return 0;
};


/**
 * legion_wmi_dev_evaluate_buffer() - Helper function for calling WMI methods that
 * return an buffer.
 * @wdev: Pointer to the WMI device to be called.
 * @instance: Instance of the called method.
 * @method_id: WMI Method ID for the method to be called.
 * @buf: Buffer of all arguments for the given method_id.
 * @size: Length of the buffer.
 * @retval: Pointer for the return value to be assigned.
 *
 * Calls wmidev_evaluate_method for Lenovo WMI devices that return an ACPI
 * integer. Validates the return value type and assigns the value to the
 * retval pointer.
 *
 * Return: 0 on success, or an error code.
 */
int legion_wmi_dev_evaluate_buffer(struct wmi_device *wdev, u8 instance, u32 method_id,unsigned char *buf, size_t size, u8 *retval,size_t max_retval_size){
	struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *ret_obj __free(kfree) = NULL;
	struct acpi_buffer input = { size, buf };
	acpi_status status;

	status = wmidev_evaluate_method(wdev, instance, method_id, &input,
					&output);
	if (ACPI_FAILURE(status))
		return -EIO;

	ret_obj = output.pointer;

	if (retval) {
		if (!ret_obj)
			return -ENODATA;

		if (ret_obj->type != ACPI_TYPE_BUFFER)
			return -ENXIO;

		if(ret_obj->buffer.length > max_retval_size)
		{
			return -EMSGSIZE;
		}

		memcpy(retval,ret_obj->buffer.pointer,ret_obj->buffer.length);
	}

	return 0;
}
