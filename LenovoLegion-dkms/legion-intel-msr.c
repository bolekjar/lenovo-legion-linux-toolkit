// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 *
 * Intel CPU voltage offsets (OC mailbox MSR 0x150), a module independent of the
 * notebook (loaded by /etc/modules-load.d/lenovo-legion.conf), refused on a CPU
 * without the OC mailbox. The attributes are in legion-intel-msr-sysfs.c.
 *
 * After a resume the offsets written by the driver are checked and written again
 * when the CPU / firmware reset them.
 */

#include "legion-intel-msr.h"
#include "legion-intel-msr-sysfs.h"

#include <asm/cpu_device_id.h>
#include <asm/intel-family.h>
#include <asm/msr.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/version.h>

/*
 * OC mailbox (MSR 0x150), common for the package: every command is done on CPU 0 from the process context (an IPI
 * per MSR access, no busy wait with the interrupts disabled), serialized by priv->lock.
 * [63] busy (set to start a command, cleared when done), [47:40] plane, [39:32] command,
 * [31:21] voltage offset (11-bit signed, 1/1024 V)
 */
#define MSR_OC_MAILBOX 0x150
#define MSR_OC_MAILBOX_BUSY BIT_ULL(63)
#define MSR_OC_MAILBOX_CMD_READ_OFFSET 0x10
#define MSR_OC_MAILBOX_CMD_WRITE_OFFSET 0x11
#define MSR_OC_MAILBOX_POLLS 100 /* 10..20 us each */

#define MSR_OC_MAILBOX_CPU 0

/*
 * Status of a finished command: bits [39:32] (evaluated, logged only for now)
 */
#define MSR_OC_MAILBOX_STATUS(value) ((u8)(((value) >> 32) & 0xFF))

static bool allow_overvolt;
module_param(allow_overvolt, bool, 0444);
MODULE_PARM_DESC(allow_overvolt,
		 "Allow positive voltage offsets (overvolt) up to +150 mV (default: undervolt only)");

/*
 * Offset in uV to the format of the mailbox: 11-bit signed integer in 1/1024 V units (two's complement, truncated
 * toward zero), written to bits [31:21]
 */
static u32 uv_to_msr(int uv)
{
	const s32 offset_units = (uv * 1024) / 1000000;

	return (u32)offset_units & 0x7FF;
}

/*
 * Convert MSR format back to microvolts
 * MSR format: 11-bit signed integer in 1/1024 V units, bits [31:21] of the mailbox
 */
static int msr_to_uv(const u32 msr_value)
{
	const s32 offset_units = sign_extend32((msr_value >> 21) & 0x7FF, 10);

	/* 1/1024 V units to uV */
	return (offset_units * 1000000) / 1024;
}

/*
 * Offset within the limits of the plane
 */
static int validate_offset(const struct legion_intel_msr_private *priv,
			   enum legion_intel_msr_plane plane, int offset_uv)
{
	if (plane < 0 || plane >= LEGION_INTEL_MSR_PLANES)
		return -EINVAL;

	if (offset_uv < priv->plane_limits[plane].min_uv ||
	    offset_uv > priv->plane_limits[plane].max_uv)
		return -EINVAL;

	return 0;
}

static int mailbox_read(u64 *value)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 16, 0)
	return rdmsrq_safe_on_cpu(MSR_OC_MAILBOX_CPU, MSR_OC_MAILBOX, value);
#else
	return rdmsrl_safe_on_cpu(MSR_OC_MAILBOX_CPU, MSR_OC_MAILBOX, value);
#endif
}

static int mailbox_write(const u64 value)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 16, 0)
	return wrmsrq_safe_on_cpu(MSR_OC_MAILBOX_CPU, MSR_OC_MAILBOX, value);
#else
	return wrmsrl_safe_on_cpu(MSR_OC_MAILBOX_CPU, MSR_OC_MAILBOX, value);
#endif
}

/*
 * Wait until the mailbox clears the busy bit, the last read value is returned in value
 */
static int mailbox_wait_ready(u64 *value)
{
	for (int poll = 0; poll < MSR_OC_MAILBOX_POLLS; poll++) {
		const int err = mailbox_read(value);

		if (err)
			return err;

		if (!(*value & MSR_OC_MAILBOX_BUSY))
			return 0;

		usleep_range(10, 20);
	}

	return -ETIMEDOUT;
}

/*
 * Status of the finished commands (bits [39:32]): logged when it changes, to find out the codes of the CPUs
 * (e.g. the OC lock) before they are used for errors; the callers hold the lock
 */
static void log_status(const int plane, const u8 command, const u64 value)
{
	static int last_status[2] = { -1, -1 };
	const int index = command == MSR_OC_MAILBOX_CMD_WRITE_OFFSET;
	const u8 status = MSR_OC_MAILBOX_STATUS(value);

	if (last_status[index] == status)
		return;

	last_status[index] = status;

	pr_info("legion-intel-msr: status of the %s command (plane %d): %#x\n",
		index ? "write" : "read", plane, status);
}

/*
 * One command: the previous command must be finished (one command at a time), the result is valid when the
 * mailbox clears the busy bit
 */
static int mailbox_command(const int plane, const u8 command,
			   const u32 offset_encoded, u64 *value)
{
	const u64 msr_val = MSR_OC_MAILBOX_BUSY | ((u64)(plane & 0xFF) << 40) |
			    ((u64)command << 32) |
			    ((u64)(offset_encoded & 0x7FF) << 21);
	int err = mailbox_wait_ready(value);

	if (err)
		return err;

	err = mailbox_write(msr_val);
	if (err)
		return err;

	err = mailbox_wait_ready(value);
	if (err)
		return err;

	log_status(plane, command, *value);

	return 0;
}

/*
 * Core Ultra CPUs (Meteor Lake, Arrow Lake) accept but silently ignore the writes of the uncore and analog I/O
 * offsets (the planes can be read), Raptor Lake supports the writes of all planes
 */
static const struct x86_cpu_id legion_intel_msr_no_uncore_write[] = {
	X86_MATCH_VFM(INTEL_METEORLAKE, NULL),
	X86_MATCH_VFM(INTEL_METEORLAKE_L, NULL),
	X86_MATCH_VFM(INTEL_ARROWLAKE_H, NULL),
	X86_MATCH_VFM(INTEL_ARROWLAKE, NULL),
	X86_MATCH_VFM(INTEL_ARROWLAKE_U, NULL),
	{}
};

static void read_voltage_limits(struct legion_intel_msr_private *priv)
{
	for (int i = 0; i < LEGION_INTEL_MSR_PLANES; i++) {
		priv->plane_limits[i].min_uv = -LEGION_INTEL_MSR_MAX_UNDERVOLT_UV;
		priv->plane_limits[i].max_uv =
			allow_overvolt ? LEGION_INTEL_MSR_MAX_OVERVOLT_UV : 0;
		priv->plane_limits[i].supported = true;
	}

	if (x86_match_cpu(legion_intel_msr_no_uncore_write)) {
		priv->plane_limits[LEGION_INTEL_MSR_UNCORE].supported = false;
		priv->plane_limits[LEGION_INTEL_MSR_ANALOGIO].supported = false;
	}
}

/*
 * The OC mailbox (MSR 0x150) is available on Intel CPUs only, it does not exist when its read faults
 */
static int check_msr_availability(void)
{
	u64 value = 0;

	if (boot_cpu_data.x86_vendor != X86_VENDOR_INTEL ||
	    boot_cpu_data.x86 < 6)
		return -ENODEV;

	if (mailbox_read(&value))
		return -ENODEV;

	return 0;
}

/**
 * legion_intel_msr_read_offset() - Read the voltage offset of a plane
 * @priv: State of the mailbox.
 * @plane: The plane.
 * @offset_uv: The offset in uV.
 *
 * Context: Takes priv->lock.
 * Return: 0 or an error of the mailbox.
 */
int legion_intel_msr_read_offset(struct legion_intel_msr_private *priv,
				 enum legion_intel_msr_plane plane,
				 int *offset_uv)
{
	u64 value = 0;
	int err;

	if (plane < 0 || plane >= LEGION_INTEL_MSR_PLANES)
		return -EINVAL;

	guard(mutex)(&priv->lock);

	err = mailbox_command(plane, MSR_OC_MAILBOX_CMD_READ_OFFSET, 0, &value);
	if (err)
		return err;

	*offset_uv = msr_to_uv((u32)value);

	return 0;
}

/**
 * legion_intel_msr_write_offset() - Write the voltage offset of a plane and verify it
 * @priv: State of the mailbox.
 * @plane: The plane.
 * @offset_uv: The offset in uV.
 *
 * The offset of the plane is common for the whole package (the same as intel-undervolt): one command on CPU 0.
 * The mailbox accepts the command also when the offset is locked (BIOS undervolt protection, OC lock) and
 * ignores it silently: the offset is read back, a different value is an error.
 *
 * Context: Takes priv->lock.
 * Return: 0, -EINVAL out of the limits, -EOPNOTSUPP for a plane which can not be written, -EIO for an offset
 * ignored by the CPU or an error of the mailbox.
 */
int legion_intel_msr_write_offset(struct legion_intel_msr_private *priv,
				  enum legion_intel_msr_plane plane,
				  int offset_uv)
{
	const u32 offset_encoded = uv_to_msr(offset_uv);
	u64 value = 0;
	u32 read_back;
	int err;

	guard(mutex)(&priv->lock);

	/* Validate plane first, it is used as index below */
	err = validate_offset(priv, plane, offset_uv);
	if (err)
		return err;

	if (!priv->plane_limits[plane].supported)
		return -EOPNOTSUPP;

	err = mailbox_command(plane, MSR_OC_MAILBOX_CMD_WRITE_OFFSET,
			      offset_encoded, &value);
	if (err)
		return err;

	err = mailbox_command(plane, MSR_OC_MAILBOX_CMD_READ_OFFSET, 0, &value);
	if (err)
		return err;

	read_back = (u32)(value >> 21) & 0x7FF;
	if (read_back != (offset_encoded & 0x7FF)) {
		pr_warn_ratelimited(
			"legion-intel-msr: offset %d uV of plane %d was not applied (read back %d uV), undervolting is probably locked in the BIOS\n",
			offset_uv, plane, msr_to_uv(read_back << 21));
		return -EIO;
	}

	priv->written[plane] = true;
	priv->written_units[plane] = offset_encoded & 0x7FF;

	return 0;
}

/**
 * legion_intel_msr_restore_offset() - Write the offset of a plane again when it was lost
 * @priv: State of the mailbox.
 * @plane: The plane.
 * @offset_uv: The restored offset in uV.
 *
 * After a resume: some CPUs / firmware reset the offsets of the OC mailbox (others keep them). Only a plane written
 * by the driver is checked, its offset is written again only when it differs, and verified.
 *
 * Context: Takes priv->lock.
 * Return: 1 restored, 0 nothing to do, -EIO not applied or an error of the mailbox.
 */
int legion_intel_msr_restore_offset(struct legion_intel_msr_private *priv,
				    enum legion_intel_msr_plane plane,
				    int *offset_uv)
{
	u64 value = 0;
	u32 units;
	int err;

	if (plane < 0 || plane >= LEGION_INTEL_MSR_PLANES)
		return -EINVAL;

	guard(mutex)(&priv->lock);

	if (!priv->written[plane] || !priv->plane_limits[plane].supported)
		return 0;

	units = priv->written_units[plane];

	err = mailbox_command(plane, MSR_OC_MAILBOX_CMD_READ_OFFSET, 0, &value);
	if (err)
		return err;

	pr_debug("legion-intel-msr: resume: plane %d written %#x, mailbox %#llx\n",
		 plane, units, value);

	if (((u32)(value >> 21) & 0x7FF) == units)
		return 0;

	err = mailbox_command(plane, MSR_OC_MAILBOX_CMD_WRITE_OFFSET, units,
			      &value);
	if (err)
		return err;

	err = mailbox_command(plane, MSR_OC_MAILBOX_CMD_READ_OFFSET, 0, &value);
	if (err)
		return err;

	if (((u32)(value >> 21) & 0x7FF) != units)
		return -EIO;

	*offset_uv = msr_to_uv(units << 21);

	return 1;
}

/**
 * legion_intel_msr_init() - Check the OC mailbox and set the limits of the planes
 * @priv: State of the mailbox.
 *
 * Return: 0 or -ENODEV without the OC mailbox.
 */
int legion_intel_msr_init(struct legion_intel_msr_private *priv)
{
	int err;

	mutex_init(&priv->lock);

	read_voltage_limits(priv);

	err = check_msr_availability();
	if (err)
		mutex_destroy(&priv->lock);

	return err;
}

void legion_intel_msr_exit(struct legion_intel_msr_private *priv)
{
	mutex_destroy(&priv->lock);
}

/*
 * After a resume (suspend to RAM, hibernation): the offsets written by the driver
 * are written again when they were reset, user space gets the uevent of a
 * restored offset
 */
static int legion_intel_msr_pm_notify(struct notifier_block *nb,
				      unsigned long action, void *data)
{
	struct legion_intel_msr_data *msr_data =
		container_of(nb, struct legion_intel_msr_data, pm_notifier);
	int offset_uv = 0;
	int ret;

	if (action != PM_POST_SUSPEND && action != PM_POST_HIBERNATION &&
	    action != PM_POST_RESTORE)
		return NOTIFY_DONE;

	dev_dbg(msr_data->fw_attr_dev, "checking the offsets after the resume\n");

	for (int plane = 0; plane < LEGION_INTEL_MSR_PLANES; plane++) {
		ret = legion_intel_msr_restore_offset(&msr_data->msr, plane,
						      &offset_uv);
		if (ret < 0) {
			dev_warn(msr_data->fw_attr_dev,
				 "offset of plane %d was reset by the resume and can not be restored: %d\n",
				 plane, ret);
		} else if (ret > 0) {
			dev_info(msr_data->fw_attr_dev,
				 "offset %d uV of plane %d restored after the resume\n",
				 offset_uv, plane);
			legion_intel_msr_sysfs_offset_restored(msr_data, plane,
							       offset_uv);
		}
	}

	return NOTIFY_OK;
}

/*
 * Data of the module: one instance (a module without a device), found by
 * legion_intel_msr_module_exit()
 */
static struct legion_intel_msr_data *legion_intel_msr_data;

static int __init legion_intel_msr_module_init(void)
{
	struct legion_intel_msr_data *data;
	int err;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	err = legion_intel_msr_init(&data->msr);
	if (err) {
		pr_info("legion-intel-msr: Intel OC mailbox (MSR 0x150) not available: %d\n",
			err);
		goto err_free;
	}

	err = legion_intel_msr_sysfs_init(data);
	if (err)
		goto err_msr_exit;

	data->pm_notifier.notifier_call = legion_intel_msr_pm_notify;
	err = register_pm_notifier(&data->pm_notifier);
	if (err)
		goto err_sysfs_exit;

	legion_intel_msr_data = data;

	return 0;

err_sysfs_exit:
	legion_intel_msr_sysfs_exit(data);
err_msr_exit:
	legion_intel_msr_exit(&data->msr);
err_free:
	kfree(data);
	return err;
}

static void __exit legion_intel_msr_module_exit(void)
{
	struct legion_intel_msr_data *data = legion_intel_msr_data;

	unregister_pm_notifier(&data->pm_notifier);
	legion_intel_msr_sysfs_exit(data);
	legion_intel_msr_exit(&data->msr);
	kfree(data);
}

module_init(legion_intel_msr_module_init);
module_exit(legion_intel_msr_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jaroslav Bolek");
MODULE_DESCRIPTION(
	"Intel CPU voltage offsets (OC mailbox MSR 0x150), part of Lenovo Legion Linux Toolkit");
MODULE_VERSION(LEGION_VERSION);
