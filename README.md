
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

1) Download https://github.com/bolekjar/lenovo-legion-linux-toolkit/archive/refs/tags/release/1.0.0.tar.gz
3) Go to LenovoLegion-release-1.0.0/LenovoLegion-dkms
4) run make
5) Kernel module is lenovo_legion.ko
6) If you want to install lenovo_legion.ko:
   - for current kernel: run "make install"
   - dkms: run "make dkms" you should see in dmesg for sucessfully installed and detected supported laptop this:

                    [    3.864415] lenovo-legion VPC2004:00:        Legion firmware attributes was initialized
                    [    3.864416] lenovo-legion VPC2004:00:        Legion data structure was initialized
                    [    3.864417] lenovo-legion VPC2004:00:        DMI Machine Information start
                    [    3.864417] lenovo-legion VPC2004:00:                 bios_vendor:LENOVO
                    [    3.864418] lenovo-legion VPC2004:00:                 bios_version:Q7CN45WW
                    [    3.864418] lenovo-legion VPC2004:00:                 bios_date:12/02/2025
                    [    3.864419] lenovo-legion VPC2004:00:                 bios_release:1.45
                    [    3.864419] lenovo-legion VPC2004:00:                 product_name:83F5
                    [    3.864419] lenovo-legion VPC2004:00:                 product_family:Legion Pro 7 16IAX10H
                    [    3.864420] lenovo-legion VPC2004:00:                 product_version:Legion Pro 7 16IAX10H
                    [    3.864420] lenovo-legion VPC2004:00:                 product_serial:XXXXXXX
                    [    3.864421] lenovo-legion VPC2004:00:                 product_sku:LENOVO_MT_83F5_BU_idea_FM_Legion Pro 7 16IAX10H
                    [    3.864421] lenovo-legion VPC2004:00:                 product_uuid:XXXXXXX
                    [    3.864421] lenovo-legion VPC2004:00:                 board_name:XXXXXXX
                    [    3.864422] lenovo-legion VPC2004:00:                 board_vendor:LENOVO
                    [    3.864422] lenovo-legion VPC2004:00:                 board_version: NO DPK
                    [    3.864423] lenovo-legion VPC2004:00:                 board_serial:XXXXXXX
                    [    3.864423] lenovo-legion VPC2004:00:                 board_asset_tag:NO Asset Tag
                    [    3.864423] lenovo-legion VPC2004:00:                 chassis_vendor:LENOVO
                    [    3.864424] lenovo-legion VPC2004:00:                 chassis_version:Legion Pro 7 16IAX10H
                    [    3.864424] lenovo-legion VPC2004:00:                 chassis_type:10
                    [    3.864424] lenovo-legion VPC2004:00:                 chassis_serial:XXXXXXX
                    [    3.864425] lenovo-legion VPC2004:00:                 chassis_asset_tag:NO Asset Tag
                    [    3.864425] lenovo-legion VPC2004:00:                 sys_vendor:LENOVO
                    [    3.864425] lenovo-legion VPC2004:00:                 ec_firmware_release:1.44
                    [    3.864426] lenovo-legion VPC2004:00:        DMI Machine Information end
                    [    3.864948] lenovo-legion VPC2004:00:        Sysfs Machine information was initialized
                    [    3.870986] lenovo-legion VPC2004:00:        WMI game zone driver was initialized
                    [    3.871068] lenovo-legion VPC2004:00:        WMI events driver was initialized
                    [    3.872664] lenovo-legion VPC2004:00:        WMI capability data 00 driver was initialized
                    [    3.877420] lenovo-legion VPC2004:00:        WMI capability data 01 driver was initialized
                    [    3.879898] lenovo-legion VPC2004:00:        WMI discrete data driver was initialized
                    [    3.879913] legion_wmi_other DC2A8805-3A8C-41BA-A6F7-092E0089CD3B-5: bound 362A3AFE-3D96-4665-8530-96DAD5BB300E-16 (ops legion_wmi_cd00_component_ops [lenovo_legion])
                    [    3.879927] legion_wmi_other DC2A8805-3A8C-41BA-A6F7-092E0089CD3B-5: bound 7A8F5407-CB67-4D6E-B547-39B3BE018154-12 (ops legion_wmi_cd01_component_ops [lenovo_legion])
                    [    3.879934] legion_wmi_other DC2A8805-3A8C-41BA-A6F7-092E0089CD3B-5: bound 91433B17-B7B7-4640-BB40-34C67349FBEC-14 (ops legion_wmi_dd_component_ops [lenovo_legion])
                    [    3.880001] lenovo-legion VPC2004:00:        WMI other driver was initialized
                    [    3.881305] lenovo-legion VPC2004:00:        WMI Fan table driver was initialized
                    [    3.881325] legion_wmi_fan_method 92549549-4BDE-4F06-AC04-CE8BF898DBAA-3: bound 87FB2A6D-D802-48E7-9208-4576C5F5C8D8-10 (ops legion_wmi_ftable_component_ops [lenovo_legion])
                    [    3.881427] lenovo-legion VPC2004:00:        WMI Fan method driver was initialized
                    [    3.881469] lenovo-legion VPC2004:00:        HwMon driver was initialized
                    [    3.881477] lenovo-legion VPC2004:00:        RAPL driver was initialized
                    [    3.881487] lenovo-legion VPC2004:00:        RAPL MMIO driver was initialized
                    [    3.882145] lenovo-legion VPC2004:00:        Intel MSR driver was initialized
                    [    3.882153] lenovo-legion VPC2004:00:        RAPL MMIO synchronization only
                    [    3.882155] lenovo-legion VPC2004:00: Legion platform driver was loaded

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
20) run systemctl start lenovo-legion-daemon.service and systemctl start lenovo-legion-daemon.service to enable this service
21) restart
22) run gui with /opt/LenovoLegion/LenovoLegion

Arch/Manjaro/EndeavourOS
- download package from aur or install with yay utility
