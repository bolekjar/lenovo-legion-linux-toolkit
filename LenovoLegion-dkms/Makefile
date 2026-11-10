SHELL := /bin/bash
KERNEL_VERSION  ?= $(shell uname --kernel-release)
KSRC := /lib/modules/$(KERNEL_VERSION)/build
VERSION := 0.0.0

INSTALL_DIR := /lib/modules/$(KERNEL_VERSION)/kernel/drivers/platform/x86
MOD_DEST_DIR := /lib/modules/$(KERNEL_VERSION)/kernel/drivers/platform/x86
DKMS_DIR := /usr/src/LenovoLegion-$(VERSION)

# Built in the source directory (the same as DKMS builds it): kbuild supports a separate output directory (MO=) since
# kernel 6.13 only, the minimal supported kernel is 6.12
obj-m := lenovo_legion.o
lenovo_legion-y := legion-dkms.o legion-firmware-attributes-class.o \
					  legion-wmi-helpers.o legion-wmi-events.o legion-wmi-gamezone.o legion-wmi-gamezone-sysfs.o legion-wmi-capdata01.o legion-wmi-ddata.o legion-wmi-other.o \
					  legion-wmi-other-sysfs.o legion-hwmon.o legion-wmi-ftable.o legion-wmi-fm.o legion-wmi-fm-sysfs.o legion-wmi-capdata00.o

# Intel CPU voltage offsets, independent of the notebook
obj-m += legion_intel_msr.o
legion_intel_msr-y := legion-intel-msr.o legion-intel-msr-sysfs.o
all:
	$(MAKE) -C $(KSRC) M=$(CURDIR) modules

allWarn:
	$(MAKE) -C $(KSRC) M=$(CURDIR) W=1 modules

clean:
	$(MAKE) -C $(KSRC) M=$(CURDIR) clean
	# Output directory of the former build with MO= (until 2026-11), not used any more
	rm --force --recursive $(CURDIR)/build

install: all
	@rm --force --verbose $(INSTALL_DIR)/lenovo_legion.ko $(INSTALL_DIR)/legion_intel_msr.ko
	@mkdir --parent --verbose $(MOD_DEST_DIR)
	@install --preserve-timestamps -D --mode=644 $(CURDIR)/lenovo_legion.ko $(INSTALL_DIR)
	@install --preserve-timestamps -D --mode=644 $(CURDIR)/legion_intel_msr.ko $(INSTALL_DIR)
	@depmod --all $(KERNEL_VERSION)
	@printf "%s\n" "Installation finished."

uninstall:
	@rm -f $(INSTALL_DIR)/lenovo_legion.ko $(INSTALL_DIR)/legion_intel_msr.ko
	@depmod --all
	@printf "%s\n" "Uninstall finished."

dkms: clean
	if [ -d $(DKMS_DIR) ];\
	then\
	 	rm --recursive $(DKMS_DIR)/*;\
		cp --recursive * $(DKMS_DIR)/;\
	else\
		mkdir --verbose $(DKMS_DIR);\
	        cp --recursive * $(DKMS_DIR);\
		dkms add -m LenovoLegion -v ${VERSION};\
	fi;
	dkms build LenovoLegion -v $(VERSION) --force
	dkms install LenovoLegion -v $(VERSION) --force
	printf "%s\n" "Loading module"
	modprobe lenovo_legion
	modprobe legion_intel_msr
	dmesg --ctime | grep "lenovo_legion\|legion-intel-msr"

uninstall-dkms:
	dkms remove -m LenovoLegion -v ${VERSION}
	rm --force --recursive $(DKMS_DIR)
