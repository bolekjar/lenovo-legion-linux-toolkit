#![allow(dead_code)]

use std::fmt::Display;
use crate::utils::*;

static SYSFS_GAMEZONE_CONTROLS: &str = "/sys/class/legion-firmware-attributes/legion-wmi-gamezone-0/attributes/";

#[derive(Debug,Default,Clone)]
pub struct Attribute {
    name: String,

    current_value: bool,
    display_name: String,
    supported: bool,
}


#[derive(Debug,Default,Clone)]
pub struct SmartFan {
    current_value: u32,
    display_name: String,
    extreme_supported: bool,
    supported: bool,
}

#[derive(Debug,Default,Clone)]
pub struct ThermalMode {
    current_value: u32,
}

#[derive(Debug,Default,Clone)]
pub struct IsACFitForOc {
    current_value: bool,
}

impl Attribute {
    pub fn new(attr_name: String) -> Result<Attribute,std::io::Error> {

        let mut attr = Attribute {
            name : attr_name,
          ..Attribute::default()
        };

        Attribute::refresh(&mut attr)?;

        Ok(attr)
    }

    pub fn refresh(&mut self) -> Result<&Attribute, std::io::Error> {
        self.current_value = read_value_as_type::<u32>(format!("{}{}/current_value",SYSFS_GAMEZONE_CONTROLS,self.name))? != 0;
        self.display_name = read_value_as_type::<String>(format!("{}{}/display_name",SYSFS_GAMEZONE_CONTROLS,self.name))?;
        self.supported = read_value_as_type::<u32>(format!("{}{}/supported",SYSFS_GAMEZONE_CONTROLS,self.name))? != 0;
        Ok(self)
    }

    pub fn set_value(&self, value: bool) -> Result<(),std::io::Error> {
        write_value_as_type(format!("{}{}/current_value", SYSFS_GAMEZONE_CONTROLS,self.name), value as u32)
    }

}

impl SmartFan {
    pub fn new() -> Result<SmartFan,std::io::Error> {

        let mut others_control = SmartFan {
            ..Self::default()
        };

        SmartFan::refresh(&mut others_control)?;

        Ok(others_control)
    }

    pub fn refresh (&mut self) -> Result<&SmartFan,std::io::Error> {
        self.current_value = read_value_as_type::<u32>(format!("{}smart_fan/current_value",SYSFS_GAMEZONE_CONTROLS))?;
        self.extreme_supported = read_value_as_type::<u32>(format!("{}smart_fan/supported",SYSFS_GAMEZONE_CONTROLS))? != 0;
        self.display_name = read_value_as_type::<String>(format!("{}smart_fan/display_name",SYSFS_GAMEZONE_CONTROLS))?;
        self.supported = read_value_as_type::<u32>(format!("{}smart_fan/supported",SYSFS_GAMEZONE_CONTROLS))? != 0;
        Ok(self)
    }

    pub fn set_value(&self, value: u32) -> Result<(),std::io::Error> {
        write_value_as_type(format!("{}smart_fan/current_value", SYSFS_GAMEZONE_CONTROLS), value)
    }
}


impl ThermalMode {
    pub fn new() -> Result<ThermalMode,std::io::Error> {

        let mut thermal_mode = ThermalMode {
            ..Self::default()
        };

        ThermalMode::refresh(&mut thermal_mode)?;

        Ok(thermal_mode)
    }

    pub fn refresh (&mut self) -> Result<&ThermalMode,std::io::Error> {
        self.current_value = read_value_as_type(format!("{}other/get_thermal_mode",SYSFS_GAMEZONE_CONTROLS))?;
        Ok(self)
    }
}

impl IsACFitForOc {
    pub fn new() -> Result<IsACFitForOc,std::io::Error> {
        let mut is_ac_fit_for_oc = IsACFitForOc {
            .. Self::default()
        };

        IsACFitForOc::refresh(&mut is_ac_fit_for_oc)?;

        Ok(is_ac_fit_for_oc)
    }

    pub fn refresh (&mut self) -> Result<&IsACFitForOc,std::io::Error> {
        self.current_value = read_value_as_type::<u32>(format!("{}other/is_ac_fit_for_oc",SYSFS_GAMEZONE_CONTROLS))? != 0;
        Ok(self)
    }
}


impl Display for SmartFan {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        _ = write!(f,"Smart Fan:\nAttributes: [\n");
        _ = write!(f, "\tCurrent Value: {}\n\
                       \tDisplay Name: {}\n\
                       \tExtreme Supported: {}\n\
                       \tSupported: {}",
                   int_to_string_power_modes(self.current_value),
                   self.display_name,
                   self.extreme_supported,
                   self.supported
        );
        write!(f,"\n]")
    }
}

impl Display for Attribute {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        _ = write!(f,"Attributes: [\n");
        _ = write!(f, "\tDisabled: {}\n\
                       \tDisplay Name: {}\n\
                       \tSupported: {}",
                   self.current_value,
                   self.display_name,
                   self.supported
        );
        write!(f,"\n]")
    }
}

impl Display for ThermalMode {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        _ = write!(f,"Thermal Mode:\nAttributes:  [\n");
        _ = write!(f, "\tCurrent Value: {}",
                   int_to_string_power_modes(self.current_value),
        );
        write!(f,"\n]")
    }
}

impl Display for IsACFitForOc {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        _ = write!(f,"IsACFitForOC:\nAttributes: [\n");
        _ = write!(f, "\tCurrent Value: {}",self.current_value);
        write!(f,"\n]")
    }
}
