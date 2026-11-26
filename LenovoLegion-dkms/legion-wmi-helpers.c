// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Based on the lenovo-wmi-helpers of the Linux kernel (drivers/platform/x86/lenovo/wmi-helpers.c),
 * modified and extended for the Lenovo Legion Linux Toolkit:
 *   Copyright (C) 2025 Derek J. Clark <derekjohn.clark@gmail.com>
 */
#include "legion-wmi-helpers.h"

#include <linux/acpi.h>
#include <linux/cleanup.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/unaligned.h>
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
int legion_wmi_dev_evaluate_int(struct wmi_device *wdev, const u8 instance,
				const u32 method_id, unsigned char *buf,
				const size_t size, u32 *retval)
{
	struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *ret_obj __free(kfree) = NULL;
	const struct acpi_buffer input = { size, buf };
	const acpi_status status = wmidev_evaluate_method(
		wdev, instance, method_id, &input, &output);
	if (ACPI_FAILURE(status))
		return -EIO;

	ret_obj = output.pointer;

	if (retval) {
		if (!ret_obj)
			return -ENODATA;

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
int legion_wmi_dev_evaluate_string(struct wmi_device *wdev, const u8 instance,
				   const u32 method_id, unsigned char *buf,
				   const size_t size, char *retval,
				   const size_t retval_size)
{
	struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *ret_obj __free(kfree) = NULL;
	const struct acpi_buffer input = { size, buf };
	const acpi_status status = wmidev_evaluate_method(
		wdev, instance, method_id, &input, &output);
	if (ACPI_FAILURE(status))
		return -EIO;

	ret_obj = output.pointer;

	if (retval) {
		if (!ret_obj)
			return -ENODATA;

		if (ret_obj->type != ACPI_TYPE_STRING)
			return -ENXIO;

		if (ret_obj->string.length >= retval_size)
			return -EMSGSIZE;

		/*
		 * The size of strscpy() includes the terminating NUL (length would drop the last character),
		 * the string fits: length < retval_size
		 */
		strscpy(retval, ret_obj->string.pointer, retval_size);
	}

	return 0;
};

/**
 * legion_wmi_dev_evaluate_buffer() - Helper function for calling WMI methods that
 * return a buffer.
 * @wdev: Pointer to the WMI device to be called.
 * @instance: Instance of the called method.
 * @method_id: WMI Method ID for the method to be called.
 * @buf: Buffer of all arguments for the given method_id.
 * @size: Length of the buffer.
 * @retval: Buffer for the returned data (NULL: the result is not used).
 * @max_retval_size: Size of the retval buffer.
 * @retval_len: Number of the returned bytes (may be NULL).
 *
 * Calls wmidev_evaluate_method for Lenovo WMI devices that return an ACPI
 * buffer. Validates the return value type and copies the data to retval.
 *
 * Return: 0 on success, or an error code.
 */
int legion_wmi_dev_evaluate_buffer(struct wmi_device *wdev, const u8 instance,
				   const u32 method_id, unsigned char *buf,
				   const size_t size, u8 *retval,
				   const size_t max_retval_size,
				   size_t *retval_len)
{
	struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *ret_obj __free(kfree) = NULL;
	const struct acpi_buffer input = { size, buf };
	const acpi_status status = wmidev_evaluate_method(
		wdev, instance, method_id, &input, &output);
	if (ACPI_FAILURE(status))
		return -EIO;

	ret_obj = output.pointer;

	if (retval) {
		if (!ret_obj)
			return -ENODATA;

		if (ret_obj->type != ACPI_TYPE_BUFFER)
			return -ENXIO;

		if (ret_obj->buffer.length > max_retval_size)
			return -EMSGSIZE;

		memcpy(retval, ret_obj->buffer.pointer, ret_obj->buffer.length);

		if (retval_len)
			*retval_len = ret_obj->buffer.length;
	}

	return 0;
}

/**
 * legion_wmi_dev_evaluate_bytes() - Helper function for calling WMI methods that
 * return a buffer or an integer.
 * @wdev: Pointer to the WMI device to be called.
 * @instance: Instance of the called method.
 * @method_id: WMI Method ID for the method to be called.
 * @buf: Buffer of all arguments for the given method_id.
 * @size: Length of the buffer.
 * @retval: Buffer for the returned data.
 * @max_retval_size: Size of the retval buffer.
 * @retval_len: Number of the returned bytes.
 *
 * Calls wmidev_evaluate_method for Lenovo WMI devices that return the output
 * values either in an ACPI buffer, or in an ACPI integer for a short result
 * (e.g. the older LENOVO_CPU_METHOD). A buffer is copied as it is, an integer
 * as its 8 little endian bytes.
 *
 * Return: 0 on success, or an error code.
 */
int legion_wmi_dev_evaluate_bytes(struct wmi_device *wdev, const u8 instance,
				  const u32 method_id, unsigned char *buf,
				  const size_t size, u8 *retval,
				  const size_t max_retval_size,
				  size_t *retval_len)
{
	struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *ret_obj __free(kfree) = NULL;
	const struct acpi_buffer input = { size, buf };
	const acpi_status status = wmidev_evaluate_method(
		wdev, instance, method_id, &input, &output);
	if (ACPI_FAILURE(status))
		return -EIO;

	ret_obj = output.pointer;
	if (!ret_obj)
		return -ENODATA;

	switch (ret_obj->type) {
	case ACPI_TYPE_BUFFER:
		if (ret_obj->buffer.length > max_retval_size)
			return -EMSGSIZE;

		memcpy(retval, ret_obj->buffer.pointer, ret_obj->buffer.length);
		*retval_len = ret_obj->buffer.length;
		return 0;
	case ACPI_TYPE_INTEGER:
		if (sizeof(u64) > max_retval_size)
			return -EMSGSIZE;

		put_unaligned_le64(ret_obj->integer.value, retval);
		*retval_len = sizeof(u64);
		return 0;
	default:
		return -ENXIO;
	}
}

/**
 * legion_wmi_block_read() - An instance of a WMI data block as the bytes of its class
 * @obj: The ACPI object of the instance (wmidev_block_query()), NULL: no data.
 * @data: Receives the instance, laid out as the buffer of the firmware.
 * @size: Size of data.
 * @fields: Sizes of the fields of the class in bytes (binary MOF: uint8 1, uint16 2, uint32 4), in their order;
 *          NULL for a class with arrays (a buffer only).
 * @count: Number of the fields.
 *
 * The firmware returns an instance as a buffer laid out as the class (e.g. Legion Pro 7 16IAX10H) or as a package of
 * its fields (e.g. the discrete data of an AMD Legion of the generation 8: Package (2) { ID, value }). Windows maps
 * both to the class, so the same firmware works with Lenovo Legion Toolkit: a buffer is copied, a package of integers
 * is written field by field (little endian, one after the other as in the buffer).
 *
 * Return: 0 on success, -ENODATA without data, -EINVAL for another object or another layout.
 */
int legion_wmi_block_read(const union acpi_object *obj, void *data,
			  const size_t size, const u8 *fields,
			  const size_t count)
{
	u8 *out = data;
	size_t offset = 0;

	if (!obj)
		return -ENODATA;

	if (obj->type == ACPI_TYPE_BUFFER) {
		if (obj->buffer.length < size)
			return -EINVAL;

		memcpy(data, obj->buffer.pointer, size);
		return 0;
	}

	if (!fields || obj->type != ACPI_TYPE_PACKAGE ||
	    obj->package.count != count)
		return -EINVAL;

	for (size_t i = 0; i < count; ++i) {
		const union acpi_object *element = &obj->package.elements[i];
		u64 value;

		if (element->type != ACPI_TYPE_INTEGER ||
		    offset + fields[i] > size)
			return -EINVAL;

		value = element->integer.value;

		switch (fields[i]) {
		case sizeof(u8):
			out[offset] = (u8)value;
			break;
		case sizeof(u16):
			put_unaligned_le16((u16)value, out + offset);
			break;
		case sizeof(u32):
			put_unaligned_le32((u32)value, out + offset);
			break;
		default:
			return -EINVAL;
		}

		offset += fields[i];
	}

	return offset == size ? 0 : -EINVAL;
}
