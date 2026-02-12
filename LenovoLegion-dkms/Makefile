SHELL := /bin/bash
KERNEL_VERSION  ?= $(shell uname --kernel-release)
KSRC := /lib/modules/$(KERNEL_VERSION)/build
VERSION := 2.0.0

INSTALL_DIR := /lib/modules/$(KERNEL_VERSION)/kernel/drivers/platform/x86
MODDEST_DIR := /lib/modules/$(KERNEL_VERSION)/kernel/drivers/platform/x86
DKMS_DIR := /usr/src/LenovoLegion-$(VERSION)

obj-m := lenovo_legion.o 
lenovo_legion-objs := legion-dkms.o legion-common.o legion-firmware-attributes-class.o legion-compatibility.o legion-machine-information.o legion-machine-information-sysfs.o \
					  legion-wmi-helpers.o legion-wmi-events.o legion-wmi-gamezone.o legion-wmi-gamezone-sysfs.o legion-wmi-capdata01.o legion-wmi-ddata.o legion-wmi-other.o \
					  legion-wmi-other-sysfs.o legion-hwmon.o legion-wmi-ftable.o legion-wmi-fm.o legion-wmi-fm-sysfs.o legion-rapl-mmio.o \
					  legion-rapl-mmio.o legion-intel-msr.o legion-intel-msr-sysfs.o legion-rapl.o legion-wmi-capdata00.o
all:
	$(MAKE) -C $(KSRC) M=$(shell pwd) modules

allWarn:
	$(MAKE) -C $(KSRC) M=$(shell pwd) KCFLAGS=-W modules

clean:
	$(MAKE) -C $(KSRC) M=$(shell pwd) clean

install: all
	@rm --force --verbose $(INSTALL_DIR)/lenovo_legion.ko
	@mkdir --parent --verbose $(MODDEST_DIR)
	@install --preserve-timestamps -D --mode=644 *.ko $(INSTALL_DIR)
	@depmod --all $(KVER)
	@printf "%s\n" "Installion finished."

uninstall:
	@rm -f $(INSTALL_DIR)/lenovo_legion.ko
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
	dmesg --ctime | grep lenovo_legion

uninstall-dkms:
	dkms remove -m LenovoLegion -v ${VERSION}
	rm --force --recursive $(DKMS_DIR)
