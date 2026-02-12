
                                               Lenovo Legion Linux Toolkit
                                                    
Unofficial control application for Legion Pro 7 tested on Gen9 and Gen 10.


This project is not affiliated with Lenovo in any way
Lenovo Legion Linux Toolkit brings additional drivers and tools for Legion Pro 7 Gen9 and Gen 10. It is the alternative to Lenovo Vantage, Legion Zone or Legion Space (Windows only).
It allows you to control features like the fan curve, power mode, power limits,  and more. This has been achieved through reverse engineering and disassembling the ACPI firmware, as well as the firmware and memory of the embedded controller (EC).


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

You will need to install the following to download and build it. If there is an error of any package, find the alternative name in your distribution and install them.

"qt6-base" "qt6-5compat" "dkms" "make" "gcc" "qt6-charts" "protobuf" "cuda" "pkgconf"

Arch/Manjaro/EndeavourOS
  sudo pacman -S "linux-headers" "qt6-base" "qt6-5compat" "dkms" "make" "gcc" "qt6-charts"  
  sudo pacman -S dkms openssl mokutil

Build

For all distribution

1) Download TODO
2) Go to LenovoLegion-release-1.0.0/LenovoLegion-dkms
3) run make
4) Kernel module is lenovo_legion.ko
5) If you want to install lenovo_legion.ko:
   - for current kernel: run "make install"
   - dkms: run "make dkms" you should see in dmesg for sucessfully installed and detected supported laptop this:
     
                   kernel: lenovo-legion VPC2004:00: Legion Pro 7 16IRX9H platform driver probing:
                   kernel: lenovo-legion VPC2004:00:         Read identifying information: DMI_SYS_VENDOR: LENOVO; DMI_PRODUCT_NAME: 83DE; DMI_BIOS_VERSION:N2CN27WW
                   kernel: lenovo-legion VPC2004:00:         Supported model
                   kernel: lenovo-legion VPC2004:00:         Platform profile interface was created
                   kernel: lenovo-legion VPC2004:00:         WMI interface was created
                   kernel: lenovo-legion VPC2004:00:         SysFs interface was created
                   kernel: lenovo-legion VPC2004:00:         HWMon interface was created
                   kernel: lenovo-legion VPC2004:00: Legion Pro 7 16IRX9H platform driver was loaded   

7) Go to LenovoLegion-release-2.0.0/LenovoLegion
8) run (qmake or qmake6) && make
9) The application is in folder LenovoLegion-release-1.0.0/LenovoLegion/Installation
10) Copy this folder to /opt/LenovoLegion
11) Copy LenovoLegion-release-1.0.0/LenovoLegion/LenovoLegion-Daemon/lenovo-legion-daemon.service to /usr/lib/systemd/system
12) run systemctl start lenovo-legion-daemon.service and systemctl start lenovo-legion-daemon.service to enable this service
13) run gui with /opt/LenovoLegion/LenovoLegion

Arch/Manjaro/EndeavourOS
- download package from aur or install with yay utility
