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

/// Attributes not provided by the model (firmware) are None
#[derive(Debug,Clone,Default)]
pub struct PowerControl {
    cpu_clp_limit: Option<Attributes>,
    cpu_ltp_limit: Option<Attributes>,
    cpu_pl1_tau: Option<Attributes>,
    gpu_to_cpu_dynamic_boost: Option<Attributes>,
    cpu_tmp_limit: Option<Attributes>,
    cpu_pp_limit: Option<Attributes>,
    cpu_stp_limit: Option<Attributes>,
    gpu_total_onac: Option<Attributes>,
    gpu_power_boost : Option<Attributes>,
    gpu_tmp_limit : Option<Attributes>,
    gpu_configurable_tgp: Option<Attributes>
}

impl PowerControl {
    pub fn new() -> Result<PowerControl,std::io::Error> {
        let cpu_power = PowerControl {
            cpu_tmp_limit : Attributes::new_optional("cpu_tmp_limit")?,
            cpu_clp_limit : Attributes::new_optional("cpu_clp_limit")?,
            cpu_ltp_limit : Attributes::new_optional("cpu_ltp_limit")?,
            cpu_pl1_tau  :  Attributes::new_optional("cpu_pl1_tau")?,
            gpu_to_cpu_dynamic_boost : Attributes::new_optional("gpu_to_cpu_dynamic_boost")?,
            cpu_pp_limit : Attributes::new_optional("cpu_pp_limit")?,
            cpu_stp_limit : Attributes::new_optional("cpu_stp_limit")?,
            gpu_total_onac : Attributes::new_optional("gpu_total_onac")?,
            gpu_power_boost : Attributes::new_optional("gpu_power_boost")?,
            gpu_tmp_limit : Attributes::new_optional("gpu_temperature_limit")?,
            gpu_configurable_tgp : Attributes::new_optional("gpu_configurable_tgp")?
        };

        Ok(cpu_power)
    }

    pub fn refresh(&mut self) -> Result<&PowerControl,std::io::Error> {
        for attribute in [&mut self.cpu_clp_limit, &mut self.cpu_ltp_limit, &mut self.cpu_pl1_tau, &mut self.gpu_to_cpu_dynamic_boost,
                          &mut self.cpu_tmp_limit, &mut self.cpu_pp_limit, &mut self.cpu_stp_limit, &mut self.gpu_total_onac,
                          &mut self.gpu_power_boost, &mut self.gpu_tmp_limit, &mut self.gpu_configurable_tgp] {
            if let Some(attribute) = attribute {
                attribute.refresh()?;
            }
        }

        Ok(self)
    }

    pub fn cpu_clp_limit(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.cpu_clp_limit,"cpu_clp_limit")
    }

    pub fn cpu_ltp_limit(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.cpu_ltp_limit,"cpu_ltp_limit")
    }

    pub fn cpu_pl1_tau(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.cpu_pl1_tau,"cpu_pl1_tau")
    }

    pub fn gpu_to_cpu_dynamic_boost(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.gpu_to_cpu_dynamic_boost,"gpu_to_cpu_dynamic_boost")
    }

    pub fn cpu_tmp_limit(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.cpu_tmp_limit,"cpu_tmp_limit")
    }

    pub fn cpu_pp_limit(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.cpu_pp_limit,"cpu_pp_limit")
    }

    pub fn cpu_stp_limit(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.cpu_stp_limit,"cpu_stp_limit")
    }

    pub fn gpu_total_onac(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.gpu_total_onac,"gpu_total_onac")
    }

    pub fn gpu_power_boost(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.gpu_power_boost,"gpu_power_boost")
    }

    pub fn gpu_tmp_limit(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.gpu_tmp_limit,"gpu_tmp_limit")
    }

    pub fn gpu_configurable_tgp(&self) -> Result<&Attributes,std::io::Error> {
        supported(&self.gpu_configurable_tgp,"gpu_configurable_tgp")
    }
}



fn supported<'a>(attribute: &'a Option<Attributes>,name: &str) -> Result<&'a Attributes,std::io::Error> {
    attribute.as_ref().ok_or_else(|| std::io::Error::new(std::io::ErrorKind::Unsupported,format!("{} is not supported by this model",name)))
}

impl Attributes {
    /// None if the attribute does not exist (not provided by the model)
    pub fn new_optional(name: &str) -> Result<Option<Attributes>,std::io::Error> {
        if !Path::new(&format!("{}{}", SYSFS_POWER_CONTROLS, name)).exists() {
            return Ok(None);
        }

        Attributes::new(name).map(Some)
    }

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


fn display_optional(attribute: &Option<Attributes>) -> String {
    match attribute {
        Some(attribute) => attribute.to_string(),
        None => String::from("N/A (not supported by this model)")
    }
}

impl Display for PowerControl {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        _ = write!(f,"CPU Power: [\n");
        _ = write!(f, "CPU CLP Limit: {},\nCPU LTP Limit: {},\nCPU PL1 Tau: {},\nGPU to CPU Dynamic Boost: {},\nCPU Temp Limit: {},\nCPU PP Limit: {},\nCPU STP Limit: {},\nGPU Total ONAC: {}\nGPU Power Boost: {},\nGPU Temp Limit: {},\nGPU Configurable TGP: {}",
               display_optional(&self.cpu_clp_limit),
               display_optional(&self.cpu_ltp_limit),
               display_optional(&self.cpu_pl1_tau),
               display_optional(&self.gpu_to_cpu_dynamic_boost),
               display_optional(&self.cpu_tmp_limit),
               display_optional(&self.cpu_pp_limit),
               display_optional(&self.cpu_stp_limit),
               display_optional(&self.gpu_total_onac),
               display_optional(&self.gpu_power_boost),
               display_optional(&self.gpu_tmp_limit),
               display_optional(&self.gpu_configurable_tgp));
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