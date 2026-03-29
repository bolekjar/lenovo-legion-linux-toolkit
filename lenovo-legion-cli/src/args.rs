use std::collections::BTreeSet;
use clap::{Parser, Subcommand};
use crate::sysfs_drivers::cpu_control::{CPUs, Topology, CPUX, SMT};
use crate::utils::{*};
use crate::sysfs_drivers::power_control::{PowerControl};

#[derive(Parser)]
#[command(version, about, long_about = None)]
pub struct Cli {

    /// Display If AC is fit for OC, if is true then you can switch to Custom mode
    #[arg(short, long)]
    is_ac_fit_oc: bool,

    /// Display Smart Fan
    #[arg(short, long)]
    smart_fan: bool,

    /// Display Touch Pad
    #[arg(short, long)]
    touchpad: bool,

    /// Display Win Key
    #[arg(short, long)]
    win_key: bool,

    /// Display Fan full speed
    #[arg(short, long)]
    fan_full_speed: bool,

    /// Display Thermal Mode [ Quiet, Balance, Power, Extreme, Custom ]
    #[arg(short, long)]
    mode: bool,

    /// Set Smart Fan [ Quiet, Balance, Power , Extreme, Custom ]
    #[arg(long)]
    smart_fan_set: Option<String>,

    #[arg(long)]
    /// Disable Touch Pad
    tpd: bool,

    #[arg(long)]
    /// Enable Touch Pad
    tpe: bool,

    #[arg(long)]
    /// Disable Win Key
    wkd: bool,

    #[arg(long)]
    /// Enable Win Key
    wke: bool,

    #[arg(long)]
    /// Fan full speed enable
    ffe: bool,

    /// Fan full speed disable
    #[arg(long)]
    ffd: bool,

    /// controls commands
    #[command(subcommand)]
    command: Option<Commands>,
}


#[derive(Subcommand)]
pub enum Commands {
    /// get values for CPU control
    CpuGet {
        /// display info about specific cpus, example "1-10,15,20,21-22"
        #[arg(short, long)]
        cpus: Option<String>,

        /// display topology
        #[arg(short, long)]
        topology: bool,

        /// display smt info
        #[arg(short, long)]
        smt: bool,

        /// display all cpu info
        #[arg(short, long)]
        all: bool,
    },

    /// set values for CPU control
    CpuSet {
        /// apply to cpus
        #[arg(short, long)]
        cpus: String,

        /// disable cpu
        #[arg(short, long)]
        disable: bool,

        /// enable cpu
        #[arg(short, long)]
        enable: bool,

        /// set min freq
        #[arg(long)]
        freq_min: Option<u32>,

        /// set max freq
        #[arg(long)]
        freq_max: Option<u32>,

        /// set governor
        #[arg(short, long)]
        governor: Option<String>,
    },

    /// get values for Power Control
    PwrGet {
        /// list all values
        #[arg(long)]
        all: bool,

        /// Cross load power limit [W]
        #[arg(long)]
        clp: bool,

        /// Long term power limit [W]
        #[arg(long)]
        ltp: bool,

        /// PL1 tau limit [s]
        #[arg(long)]
        pl1: bool,

        /// GPU to CPU dynamic boost [W]
        #[arg(long)]
        gpu_to_cpu_db: bool,

        /// Temperature limit [°C]
        #[arg(long)]
        tmp: bool,

        /// Peak power limit [W]
        #[arg(long)]
        pp: bool,

        /// Short term power limit [W]
        #[arg(long)]
        stp: bool,

        /// GPU total on AC [W]
        #[arg(long)]
        gpu_total_onac: bool,

        /// GPU Power boost [W]
        #[arg(long)]
        gpu_power_boost: bool,

        /// GPU Temperature limit [°C]
        #[arg(long)]
        gpu_tmp: bool,

        /// GPU Configurable TGP [W]
        #[arg(long)]
        gpu_tgp: bool
    },

    /// set values for Power Control
    PwrSet {

        /// CPU Cross load power limit [W]
        #[arg(long)]
        clp: Option<u32>,

        /// CPU Long term power limit [W]
        #[arg(long)]
        ltp: Option<u32>,

        /// CPU PL1 tau limit [s]
        #[arg(long)]
        pl1: Option<u32>,

        /// GPU to CPU dynamic boost [W]
        #[arg(long)]
        gpu_to_cpu_db: Option<u32>,

        /// CPU Temperature limit [°C]
        #[arg(long)]
        tmp: Option<u32>,

        /// CPU Peak power limit [W]
        #[arg(long)]
        pp: Option<u32>,

        /// CPU Short term power limit [W]
        #[arg(long)]
        stp: Option<u32>,

        /// GPU total on AC [W]
        #[arg(long)]
        gpu_total_onac: Option<u32>,

        /// GPU Power boost [W]
        #[arg(long)]
        gpu_power_boost: Option<u32>,

        /// GPU Temperature limit [°C]
        #[arg(long)]
        gpu_tmp: Option<u32>,

        /// GPU Configurable TGP [W]
        #[arg(long)]
        gpu_tgp: Option<u32>
    },

    ///get values for Offset Control
    OffSetGet {
        /// Display cpu package offset values [uV]
        #[arg(short, long)]
        cpu_package: bool,
    },

    ///set values for Offset Control
    OffSetSet {
        /// CPU offset [uV]
        #[arg(long)]
        cpu: Option<i32>,

        /// GPU offset [uV]
        #[arg(long)]
        gpu: Option<i32>,

        /// Analogio offset [uV]
        #[arg(long)]
        analogio: Option<i32>,

        /// Cache offset [uV]
        #[arg(long)]
        cache: Option<i32>,

        /// Uncore offset [uV]
        #[arg(long)]
        uncore: Option<i32>,
    }

}

impl Cli {
    pub fn command(&self) -> &Option<Commands> {
        &self.command
    }

    pub fn smart_fan(&self) -> bool {
        self.smart_fan
    }

    pub fn touchpad(&self) -> bool {
        self.touchpad
    }

    pub fn win_key(&self) -> bool {
        self.win_key
    }

    pub fn thermal_mode(&self) -> bool {
        self.mode
    }
    pub fn fan_full_speed(&self) -> bool { self.fan_full_speed }

    pub fn smart_fan_set(&self) -> &Option<String> {
        &self.smart_fan_set
    }
    pub fn tpd(&self) -> bool {
        self.tpd
    }
    pub fn tpe(&self) -> bool {
        self.tpe
    }
    pub fn wkd(&self) -> bool {
        self.wkd
    }
    pub fn wke(&self) -> bool {
        self.wke
    }
    pub fn is_ac_fit_oc(&self) -> bool {
        self.is_ac_fit_oc
    }
    pub fn ffe(&self) -> bool { self.ffe }
    pub fn ffd(&self) -> bool { self.ffd }
}

impl Commands {
    pub fn parse(&self) -> Result<(),std::io::Error> {
        match self {

            Commands::CpuGet {
                cpus,
                topology,
                smt,
                all
            } => {
                let mut cpus_vec: BTreeSet<u32> = BTreeSet::new();

                match cpus {
                    Some(value) => {
                        let topology = Topology::new()?;
                        cpus_vec = parse_cpu_range_value(value.as_str())?;

                        if !cpus_vec.is_subset(topology.possible()) {
                            return Err(std::io::Error::new(std::io::ErrorKind::InvalidData,"Cpu range is too big !"));
                        }
                    }
                    None => {}
                }

                if *topology
                {
                    println!("{}",Topology::new()?);
                }

                if *smt
                {
                    print!("{}",SMT::new()?);
                }

                if *all {
                    print!("{}",CPUs::new()?);
                }

                if !cpus_vec.is_empty()
                {
                    for cpu in cpus_vec {
                        print!("{}",CPUX::new(cpu)?);
                    }
                }
            }

            Commands::CpuSet {
                cpus,
                disable,
                enable,
                freq_min,
                freq_max,
                governor,
            } => {
                let cpus_vec: BTreeSet<u32> = parse_cpu_range_value(cpus.as_str())?;
                let topology = Topology::new()?;

                if !cpus_vec.is_subset(topology.possible()) {
                    return Err(std::io::Error::new(std::io::ErrorKind::InvalidData,"Cpu range is too big !"));
                }

                if *disable && *enable
                {
                    return Err(std::io::Error::new(std::io::ErrorKind::InvalidData,"Cannot set cpu disable and enable in same time !"));
                }

                for cpu in cpus_vec {
                    let cpu_x = CPUX::new(cpu)?;

                    match freq_min
                    {
                        Some(min) => {
                            cpu_x.freq_min(*min)?;
                        }

                        None => {}
                    }

                    match freq_max
                    {
                        Some(max) => {
                            cpu_x.freq_max(*max)?;
                        }

                        None => {}
                    }

                    match governor
                    {
                        Some(governor) => {
                            cpu_x.governor(governor)?;
                        }
                        None => {}
                    }

                    if *enable
                    {
                        cpu_x.enable()?;
                    }

                    if *disable
                    {
                        cpu_x.disable()?;
                    }

                }
            }

            Commands::PwrGet {
                all,
                clp,
                ltp,
                pl1,
                gpu_to_cpu_db,
                tmp,
                pp,
                stp,
                gpu_total_onac,
                gpu_power_boost,
                gpu_tmp,
                gpu_tgp
            } => {
               let cpu_power = PowerControl::new()?;

                if *all
                {
                    print!("{}",cpu_power);
                }

                if *clp
                {
                    print!("{}",cpu_power.cpu_clp_limit());
                }

                if *ltp
                {
                    print!("{}",cpu_power.cpu_ltp_limit());
                }

                if *pl1
                {
                    print!("{}",cpu_power.cpu_pl1_tau());
                }

                if *gpu_to_cpu_db
                {
                    print!("{}",cpu_power.gpu_to_cpu_dynamic_boost());
                }

                if *tmp
                {
                    print!("{}",cpu_power.cpu_tmp_limit());
                }

                if *pp
                {
                    print!("{}", PowerControl::new()?.cpu_pp_limit());
                }

                if *stp
                {
                    print!("{}", PowerControl::new()?.cpu_stp_limit());
                }

                if *gpu_total_onac
                {
                    print!("{}", PowerControl::new()?.gpu_total_onac());
                }

                if *gpu_power_boost
                {
                    print!("{}", PowerControl::new()?.gpu_power_boost());
                }

                if *gpu_tmp
                {
                    print!("{}", PowerControl::new()?.gpu_tmp_limit());
                }

                if *gpu_tgp
                {
                    print!("{}", PowerControl::new()?.gpu_configurable_tgp());
                }
            }

            Commands::PwrSet {
                clp,
                ltp,
                pl1,
                gpu_to_cpu_db,
                tmp,
                pp,
                stp,
                gpu_total_onac,
                gpu_power_boost,
                gpu_tmp,
                gpu_tgp
            } => {

                let cpu_power = PowerControl::new()?;

                match clp {
                    Some(clp) => {
                        cpu_power.cpu_clp_limit().current_value(*clp)?;
                    }
                    None => {}
                }

                match ltp {
                    Some(ltp) => {
                        cpu_power.cpu_ltp_limit().current_value(*ltp)?;
                    }
                    None => {}
                }

                match pl1 {
                    Some(pl1) => {
                        cpu_power.cpu_pl1_tau().current_value(*pl1)?;
                    }
                    None => {}
                }

                match gpu_to_cpu_db {
                    Some(gpu_to_cpu_db) => {
                        cpu_power.gpu_to_cpu_dynamic_boost().current_value(*gpu_to_cpu_db)?;
                    }
                    None => {}
                }

                match tmp {
                    Some(tmp) => {
                        cpu_power.cpu_tmp_limit().current_value(*tmp)?;
                    }
                    None => {}
                }

                match pp {
                    Some(pp) => {
                        cpu_power.cpu_pp_limit().current_value(*pp)?;
                    }
                    None => {}
                }

                match stp {
                    Some(stp) => {
                        cpu_power.cpu_stp_limit().current_value(*stp)?;
                    }
                    None => {}
                }

                match gpu_total_onac {
                    Some(gpu_total_onac) => {
                        cpu_power.gpu_total_onac().current_value(*gpu_total_onac)?;
                    }
                    None => {}
                }

                match gpu_power_boost {
                    Some(gpu_power_boost) => {
                        cpu_power.gpu_power_boost().current_value(*gpu_power_boost)?;
                    }
                    None => {}
                }

                match gpu_tmp {
                    Some(gpu_tmp) => {
                        cpu_power.gpu_tmp_limit().current_value(*gpu_tmp)?;
                    }
                    None => {}
                }

                match gpu_tgp {
                    Some(gpu_tgp) => {
                        cpu_power.gpu_configurable_tgp().current_value(*gpu_tgp)?;
                    }
                    None => {}
                }
            }

            Commands::OffSetGet {
                cpu_package
            } => {
                let offset_control = crate::sysfs_drivers::offset_control::OffsetControl::new()?;

                if *cpu_package
                {
                    print!("{}",offset_control);
                }
            }

            Commands::OffSetSet {
                cpu,
                gpu,
                analogio,
                cache,
                uncore
             } => {
                let offset_control = crate::sysfs_drivers::offset_control::OffsetControl::new()?;

                match cpu {
                    Some(cpu) => {
                        offset_control.set_cpu_offset(*cpu)?;
                    }
                    None => {}
                }

                match gpu {
                    Some(gpu) => {
                        offset_control.set_gpu_offset(*gpu)?;
                    }
                    None => {}
                }

                match analogio {
                    Some(analogio) => {
                        offset_control.set_analogio_offset(*analogio)?;
                    }
                    None => {}
                }

                match cache {
                    Some(cache) => {
                        offset_control.set_cache_offset(*cache)?;
                    }
                    None => {}
                }

                match uncore {
                    Some(uncore) => {
                        offset_control.set_uncore_offset(*uncore)?;
                    }
                    None => {}
                }
             }
        }

        Ok(())
    }
}