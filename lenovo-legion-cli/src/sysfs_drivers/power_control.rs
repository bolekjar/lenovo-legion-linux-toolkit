#![allow(dead_code)]

use std::collections::{BTreeMap, BTreeSet};
use std::fmt::Display;
use crate::utils::{*};
use std::path::Path;
use std::fs;

static  SYSFS_POWER_CONTROLS: &str = "/sys/class/legion-firmware-attributes/legion-wmi-other-0/attributes/";

#[derive(Debug,Clone,Default)]
pub struct Attributes {
    path: String,

    current_value: u32,
    default_value: BTreeMap<u32,BTreeSet<u32>>,
    display_name: String,
    max_value: BTreeMap<u32,BTreeSet<u32>>,
    min_value: BTreeMap<u32,BTreeSet<u32>>,
    scalar_increment: BTreeMap<u32,BTreeSet<u32>>,
    steps: BTreeMap<u32,BTreeSet<u32>>,
    supported: BTreeMap<u32,BTreeSet<u32>>,
    type_value: String
}

#[derive(Debug,Clone,Default)]
pub struct PowerControl {
    cpu_clp_limit: Attributes,
    cpu_ltp_limit: Attributes,
    cpu_pl1_tau: Attributes,
    gpu_to_cpu_dynamic_boost: Attributes,
    cpu_tmp_limit: Attributes,
    cpu_pp_limit: Attributes,
    cpu_stp_limit: Attributes,
    gpu_total_onac: Attributes,
    gpu_power_boost : Attributes,
    gpu_tmp_limit : Attributes,
    gpu_configurable_tgp: Attributes
}

impl PowerControl {
    pub fn new() -> Result<PowerControl,std::io::Error> {
        let cpu_power = PowerControl {
            cpu_tmp_limit : Attributes::new("cpu_tmp_limit")?,
            cpu_clp_limit : Attributes::new("cpu_clp_limit")?,
            cpu_ltp_limit : Attributes::new("cpu_ltp_limit")?,
            cpu_pl1_tau  :  Attributes::new("cpu_pl1_tau")?,
            gpu_to_cpu_dynamic_boost : Attributes::new("gpu_to_cpu_dynamic_boost")?,
            cpu_pp_limit : Attributes::new("cpu_pp_limit")?,
            cpu_stp_limit : Attributes::new("cpu_stp_limit")?,
            gpu_total_onac : Attributes::new("gpu_total_onac")?,
            gpu_power_boost : Attributes::new("gpu_power_boost")?,
            gpu_tmp_limit : Attributes::new("gpu_temperature_limit/")?,
            gpu_configurable_tgp : Attributes::new("gpu_configurable_tgp")?
        };

        Ok(cpu_power)
    }

    pub fn refresh(&mut self) -> Result<&PowerControl,std::io::Error> {
        self.cpu_clp_limit.refresh()?;
        self.cpu_ltp_limit.refresh()?;
        self.cpu_pl1_tau.refresh()?;
        self.gpu_to_cpu_dynamic_boost.refresh()?;
        self.cpu_tmp_limit.refresh()?;
        self.cpu_pp_limit.refresh()?;
        self.cpu_stp_limit.refresh()?;
        self.gpu_total_onac.refresh()?;

        Ok(self)
    }

    pub fn cpu_clp_limit(&self) -> &Attributes {
        &self.cpu_clp_limit
    }

    pub fn cpu_ltp_limit(&self) -> &Attributes {
        &self.cpu_ltp_limit
    }

    pub fn cpu_pl1_tau(&self) -> &Attributes {
        &self.cpu_pl1_tau
    }

    pub fn gpu_to_cpu_dynamic_boost(&self) -> &Attributes {
        &self.gpu_to_cpu_dynamic_boost
    }

    pub fn cpu_tmp_limit(&self) -> &Attributes {
        &self.cpu_tmp_limit
    }

    pub fn cpu_pp_limit(&self) -> &Attributes {
        &self.cpu_pp_limit
    }

    pub fn cpu_stp_limit(&self) -> &Attributes {
        &self.cpu_stp_limit
    }

    pub fn gpu_total_onac(&self) -> &Attributes {
        &self.gpu_total_onac
    }

    pub fn gpu_power_boost(&self) -> &Attributes {
        &self.gpu_power_boost
    }

    pub fn gpu_tmp_limit(&self) -> &Attributes {
        &self.gpu_tmp_limit
    }

    pub fn gpu_configurable_tgp(&self) -> &Attributes {
        &self.gpu_configurable_tgp
    }
}



impl Attributes {
    pub fn new(name: &str) -> Result<Attributes,std::io::Error> {
        let mut attributes = Attributes {
            path: format!("{}{}/", SYSFS_POWER_CONTROLS, name),
            ..Default::default()
        };

        Attributes::refresh(&mut attributes)?;

        Ok(attributes)
    }

    pub fn refresh (&mut self) -> Result<&Attributes,std::io::Error> {
        self.current_value = read_value_as_type::<u32>(self.path.clone() + "current_value")?;
        self.default_value = parse_power_map((self.path.clone() + "default_value").as_str())?;
        self.display_name = read_value_as_type::<String>(self.path.clone() + "display_name")?;
        self.max_value = parse_power_map((self.path.clone() + "max_value").as_str())?;
        self.min_value = parse_power_map((self.path.clone() + "min_value").as_str())?;
        self.scalar_increment = parse_power_map((self.path.clone() + "scalar_increment").as_str())?;
        self.steps = parse_power_map((self.path.clone() + "steps").as_str())?;
        self.supported = parse_power_map((self.path.clone() + "supported").as_str())?;
        self.type_value = read_value_as_type::<String>(self.path.clone() + "type")?;
        Ok(self)
    }

    pub fn current_value(&self,value: u32) -> Result<(),std::io::Error> {
        write_value_as_type::<u32>(self.path.clone() + "current_value", value)
    }
}


fn display_power_map(map: &BTreeMap<u32,BTreeSet<u32>>) -> String {
    if map.is_empty()
    {
        return String::from("N/A");
    }
    map.iter().map(|val | format!("{}=({})",int_to_string_power_modes(*val.0),val.1.iter().map(|val | format!("{}",val)).collect::<Vec<String>>().join("|"))).collect::<Vec<String>>().join(", ")
}


impl Display for Attributes {

    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        _ = write!(f,"Attributes: [\n");
        _ = write!(f, "\tCurrent Value: {}\n\
                       \tDefault Value: {}\n\
                       \tDisplay Name: {}\n\
                       \tMax Value: {}\n\
                       \tMin Value: {}\n\
                       \tScalar Increment: {}\n\
                       \tSteps: {}\n\
                       \tSupported: {}\n\
                       \tType Value: {}",
                   self.current_value,
                   display_power_map(&self.default_value),
                   self.display_name,
                   display_power_map(&self.max_value),
                   display_power_map(&self.min_value),
                   display_power_map(&self.scalar_increment),
                   display_power_map(&self.steps),
                   display_power_map(&self.supported),
                   self.type_value);
        write!(f,"\n]")
    }

}


impl Display for PowerControl {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        _ = write!(f,"CPU Power: [\n");
        _ = write!(f, "CPU CLP Limit: {},\nCPU LTP Limit: {},\nCPU PL1 Tau: {},\nGPU to CPU Dynamic Boost: {},\nCPU Temp Limit: {},\nCPU PP Limit: {},\nCPU STP Limit: {},\nGPU Total ONAC: {}\nGPU Power Boost: {},\nGPU Temp Limit: {},\nGPU Configurable TGP: {}",
               self.cpu_clp_limit,
               self.cpu_ltp_limit,
               self.cpu_pl1_tau,
               self.gpu_to_cpu_dynamic_boost,
               self.cpu_tmp_limit,
               self.cpu_pp_limit,
               self.cpu_stp_limit,
               self.gpu_total_onac,
               self.gpu_power_boost,
               self.gpu_tmp_limit,
               self.gpu_configurable_tgp);
        write!(f,"\n]\n")
    }
}


pub fn parse_power_map(path : &str ) -> Result<BTreeMap<u32,BTreeSet<u32>>,std::io::Error>
{
    match fs::read_to_string(Path::new(path)) {
        Ok(contents) => {
            return parse_power_map_value(contents.as_str());
        }
        Err(err) => {
            return Err(std::io::Error::new(err.kind(), format!("Reading of file {} error: {}",path,err.to_string())));
        }
    }
}