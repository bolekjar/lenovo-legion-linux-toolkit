
                                               Lenovo Legion Linux Toolkit
                                                    
Unofficial control application for Legion laptops. Tested on Gen9 and Gen 10.


This project is not affiliated with Lenovo in any way
Lenovo Legion Linux Toolkit brings additional drivers and tools for Legion laptops Gen9 and Gen 10. It is the alternative to Lenovo Vantage, Legion Zone or Legion Space (Windows only).
It allows you to control features like the fan curve, power mode, power limits,  and more. This has been achieved through reverse engineering and disassembling the ACPI firmware, as well as the firmware and memory of the embedded controller (EC).

<img width="3837" height="2120" alt="image" src="https://github.com/user-attachments/assets/4a18905d-a27c-4bdb-a1cf-cee28bf32231" />
<img width="3839" height="2120" alt="image" src="https://github.com/user-attachments/assets/085c4a67-ed7e-4292-9345-dd3392f6c2a6" />
<img width="3839" height="2120" alt="image" src="https://github.com/user-attachments/assets/bb2c9e4d-f56b-421a-85b9-dafbb45aa1db" />
<img width="3839" height="2120" alt="image" src="https://github.com/user-attachments/assets/b65df99e-32b6-406a-a92d-f9f15f73005d" />
<img width="3839" height="2120" alt="image" src="https://github.com/user-attachments/assets/a61d57b8-ced5-4ba9-8cab-96c45e8c5d74" />
<img width="3839" height="2121" alt="image" src="https://github.com/user-attachments/assets/7f3f4837-87cd-4f38-96ae-2a219e3af510" />
<img width="3839" height="2121" alt="image" src="https://github.com/user-attachments/assets/57a825bf-9fe2-4723-82ff-82fe891b27e5" />

🚀 Features

- GUI replacement for Lenovo Vantage/Space: Fan control (Fan curve), Power Control (GPU, CPU)
- Set a fully featured custom fan curve with up to 10 points
- Set temperature points for when the fan speed (level)
- Use CPU, GPU to control the fan all at the same it
- Switch power mode (quiet, balanced, performance, extreme, custom) using software
- Changing with Fn+Q is also possible
- Switch between different fan profiles depending on the power profile
- Monitor fan speeds, power consumtion and temperatures (CPU, GPU)
- Frequency Control
- CPU Control enable/disable SMT enable cores and set governor
- and more

📣 Overview
- it comes with a driver lenovo_legion (kernel module) that implements the Linux standard interfaces (sysfs, hwmon)
- it comes with a driver legion_intel_msr (separate kernel module) for the Intel CPU voltage offsets, it does not depend on the notebook (any Intel CPU with the OC mailbox)
- it comes with a LenovoLegion-Daemon (daemon under root privileges) the provide for LenovoLegion (gui part) data
- it comes with a LenovoLegion (GUI) the provide UI for the user

📦 Available Packages

Arch Base Distros:
  - 

📌 Confirmed Compatible Models
  - Legion Pro 7 16IRX9H
  - Legion Pro 7 16IAX10H

💡 Instructions

Please do the following:

  -  follow installation instructions

Requirements

Linux kernel 6.12 or newer (the kernel modules are not built for older kernels).

You will need to install the following to download and build it. If there is an error of any package, find the alternative name in your distribution and install them.

"qt6-base" "qt6-5compat" "dkms" "make" "gcc" "qt6-charts" "protobuf" "cuda" "pkgconf"

Arch/Manjaro/EndeavourOS
  sudo pacman -S "linux-headers" "qt6-base" "qt6-5compat" "dkms" "make" "gcc" "qt6-charts"  
  sudo pacman -S dkms openssl mokutil

Build

For all distribution

1) Download https://github.com/bolekjar/lenovo-legion-linux-toolkit/archive/refs/tags/release/1.0.0.tar.gz
3) Go to LenovoLegion-release-1.0.0/LenovoLegion-dkms
4) run make
5) Kernel modules are lenovo_legion.ko (Lenovo Legion notebook) and legion_intel_msr.ko (Intel CPU voltage offsets, any notebook, loaded by /etc/modules-load.d/lenovo-legion.conf)
6) If you want to install the kernel modules:
   - for current kernel: run "make install"
   - dkms: run "make dkms"
   - check the module on a supported notebook: the WMI drivers are bound and the hwmon device "legion" exists

                    for d in /sys/bus/wmi/devices/*; do echo "$(basename $d) -> $(basename "$(readlink $d/driver)")"; done | grep legion
                    grep -l '^legion$' /sys/class/hwmon/hwmon*/name

     the module is not loaded on an unsupported model (no error, the DMI model check fails)

7) Go to LenovoLegion-release-1.0.0/LenovoLegion
8) run (qmake or qmake6) && make
9) The application is in folder LenovoLegion-release-1.0.0/LenovoLegion/Installation
10) Copy this folder to /opt/LenovoLegion
11) Copy LenovoLegion-release-1.0.0/LenovoLegion/LenovoLegion-Daemon/lenovo-legion-daemon.service to /usr/lib/systemd/system
12) Copy LenovoLegion-release-1.0.0/LenovoLegion/LenovoLegion-Application/LenovoLegion.desktop to /usr/share/applications
13) Copy LenovoLegion-release-1.0.0/LenovoLegion/LenovoLegion-dkms/blacklist-lenovo-legion.conf  to /etc/modprobe.d
14) Copy LenovoLegion-release-1.0.0/LenovoLegion/LenovoLegion-dkms/lenovo-legion.conf to /etc/modules-load.d/
15) Copy LenovoLegion-release-1.0.0/LenovoLegion/LenovoLegion-dkms/99-rapl-readonly.rules to /etc/udev/rules.d
16) Copy LenovoLegion-release-1.0.0/LenovoLegion/LenovoLegion-dkms/rapl-readonly.sh  to /opt/LenovoLegion
18) chmod 755 /opt/LenovoLegion/rapl-readonly.sh
19) Copy LenovoLegion-release-1.0.0/LenovoLegion/LenovoLegion-Daemon/lenovo-legion.sysusers to /usr/lib/sysusers.d/lenovo-legion.conf, copy LenovoLegion-release-1.0.0/LenovoLegion/LenovoLegion-Daemon/lenovo-legion-setup.sh to /opt/LenovoLegion and run sudo /opt/LenovoLegion/lenovo-legion-setup.sh (adds your user to the legion group, log in again afterwards)
20) run systemctl start lenovo-legion-daemon.service and systemctl start lenovo-legion-daemon.service to enable this service
21) restart
22) run gui with /opt/LenovoLegion/LenovoLegion

Arch/Manjaro/EndeavourOS
- download package from aur or install with yay utility
