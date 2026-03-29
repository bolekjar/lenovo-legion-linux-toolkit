#![allow(dead_code)]

use std::fmt::Display;
use crate::utils::*;

static SYSFS_INTEL_MSR: &str = "/sys/class/legion-intel-msr/intel-msr-0/";

#[derive(Debug,Default,Clone)]
pub struct Attribute  {
    name: String,

    offset: i32,
    max_overvoltage:  u32,
    max_undervoltage: u32,
    supported: bool,
}

#[derive(Debug,Default,Clone)]
pub struct OffsetControl {
    analogio: Attribute,
    cache: Attribute,
    cpu: Attribute,
    gpu: Attribute,
    uncore: Attribute,
}

impl Attribute {

    pub fn new (name : String) -> Result<Attribute,std::io::Error> {

        let mut attributes = Attribute {
            name,
            ..Attribute::default()
        };

        Attribute::refresh(&mut attributes)?;

        Ok(attributes)
    }

    pub fn refresh (&mut self) -> Result<&Attribute, std::io::Error> {
        self.offset = read_value_as_type::<i32>(format!("{}{}",SYSFS_INTEL_MSR,self.name.clone() + "_offset"))?;
        self.supported = read_value_as_type::<u32>(format!("{}{}",SYSFS_INTEL_MSR,self.name.clone() + "_offset_ctrl_supported"))? != 0;
        self.max_overvoltage = read_value_as_type(format!("{}{}",SYSFS_INTEL_MSR,self.name.clone() + "_max_overvolt"))?;
        self.max_undervoltage = read_value_as_type(format!("{}{}",SYSFS_INTEL_MSR,self.name.clone() + "_max_undervolt"))?;
        Ok(self)
    }

    pub fn set_offset(&self, new_offset: i32) -> Result<(),std::io::Error> {
        write_value_as_type::<i32>(format!("{}{}",SYSFS_INTEL_MSR,self.name.clone() + "_offset"), new_offset)?;
        Ok(())
    }
}

impl OffsetControl {
    pub fn new () -> Result<OffsetControl,std::io::Error> {

        let offset_control = OffsetControl {
            analogio: Attribute::new("analogio".to_string())?,
            cache: Attribute::new("cache".to_string())?,
            cpu: Attribute::new("cpu".to_string())?,
            gpu: Attribute::new("gpu".to_string())?,
            uncore: Attribute::new("uncore".to_string())?,
        };

        Ok(offset_control)
    }

    pub fn refresh(&mut self) -> Result<&OffsetControl, std::io::Error> {
        self.analogio.refresh()?;
        self.cache.refresh()?;
        self.cpu.refresh()?;
        self.gpu.refresh()?;
        self.uncore.refresh()?;

        Ok(self)
    }

    pub fn set_cpu_offset(&self, new_offset: i32) -> Result<(),std::io::Error> {
        self.cpu.set_offset(new_offset)
    }

    pub fn set_cache_offset(&self, new_offset: i32) -> Result<(),std::io::Error> {
        self.cache.set_offset(new_offset)
    }

    pub fn set_gpu_offset(&self, new_offset: i32) -> Result<(),std::io::Error> {
        self.gpu.set_offset(new_offset)
    }

     pub fn set_analogio_offset(&self, new_offset: i32) -> Result<(),std::io::Error> {
        self.analogio.set_offset(new_offset)
    }

    pub fn set_uncore_offset(&self, new_offset: i32) -> Result<(),std::io::Error> {
        self.uncore.set_offset(new_offset)
    }
}


impl Display for Attribute {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        _ = write!(f,"Attributes: [\n");
        _ = write!(f, "\tOffset: {}\n\
                       \tMax Overvoltage: {}\n\
                       \tMax Undervoltage: {}\n\
                       \tSupported: {}",
                   { if self.supported {self.offset.to_string() } else { "N/A".to_string() } },
                   { if self.supported {self.max_overvoltage.to_string() } else { "N/A".to_string() } },
                   { if self.supported {self.max_undervoltage.to_string() } else { "N/A".to_string() } },
                   self.supported
        );
        _ =write!(f,"\n]");
        Ok(())
    }
}

impl Display for OffsetControl {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        _ = write!(f, "Offsets: [\n");
        _ = write!(f, "CPU {}\n\
                       Cache {}\n\
                       GPU {}\n\
                       AnalogIO {}\n\
                       Uncore {}",
                   self.cpu,
                   self.cache,
                   self.gpu,
                   self.analogio,
                   self.uncore
        );
        _ = write!(f, "\n]");
        Ok(())
    }
}