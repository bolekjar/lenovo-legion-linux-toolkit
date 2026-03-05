use std::collections::BTreeSet;
use std::collections::BTreeMap;
use std::fs;
use std::path::Path;
use std::str::FromStr;

pub fn parse_cpu_range_value(value : &str ) -> Result<BTreeSet<u32>,std::io::Error>
{
    let mut ret_cpus : BTreeSet<u32> = BTreeSet::new();

    if value.trim().is_empty() {
        return Ok(ret_cpus);
    }

    for value in value.trim().split(",") {
        match value.split('-').map(|x| x.trim().parse::<u32>()).collect::<Result<Vec<u32>,_>>()
        {
            Ok(cpus) => {
                let range = if cpus.len() == 1 {
                    Ok((cpus[0],cpus[0]))
                } else if cpus.len() == 2 {
                    Ok((cpus[0], cpus[1]))
                } else {
                    Err(std::io::Error::new(std::io::ErrorKind::Other,"Parse error"))
                };

                match range {
                    Ok(range) => {
                        for cpu in range.0 ..=range.1 {
                            if !ret_cpus.insert(cpu) {
                                return Err(std::io::Error::new(std::io::ErrorKind::Other,format!("CPU {} was not added !",cpu)));
                            }
                        }
                    }
                    Err(value) => return Err(std::io::Error::new(std::io::ErrorKind::Other,value))
                }
            }
            Err(value) => {
                return Err(std::io::Error::new(std::io::ErrorKind::Other,format!("Invalid value: {}",value)));
            }
        }
    }

    Ok(ret_cpus)
}

pub fn parse_power_map_value(value : &str ) -> Result<BTreeMap<u32,BTreeSet<u32>>,std::io::Error>
{
    let mut ret_map : BTreeMap<u32,BTreeSet<u32>> = BTreeMap::new();

    if value.trim().is_empty() {
        return Ok(ret_map);
    }

    for value in value.trim().split(",") {
        match value.split('=').map(|x| x.trim().parse::<String>()).collect::<Result<Vec<String>,_>>()
        {
            Ok(cpus) => {
                if cpus.len() == 2 {
                    match cpus[0].parse::<u32>() {
                        Ok(cpu) => {
                            match cpus[1].split("|").map(|x| x.trim().parse::<u32>()).collect::<Result<BTreeSet<u32>,_>>(){
                                Ok(values) => {
                                     if !ret_map.insert(cpu,values).is_none() {
                                        return Err(std::io::Error::new(std::io::ErrorKind::Other,format!("CPU {} was not added !",cpu)));
                                    }
                                }
                                Err(_) => {}
                            }
                        }
                        Err(_) => return Err(std::io::Error::new(std::io::ErrorKind::Other,format!("Invalid CPU value: {}",cpus[0])))
                    }
                }
            }
            Err(value) => {
                return Err(std::io::Error::new(std::io::ErrorKind::Other,format!("Invalid value: {}",value)));
            }
        }
    }

    Ok(ret_map)
}

pub fn read_value_as_type<Type: FromStr>(path: String) -> Result<Type, std::io::Error>
{
    match fs::read_to_string(&path)
    {
        Ok(content) => {
            match content.trim().parse::<Type>() {
                Ok(value) => {
                    Ok(value)
                }
                Err(_err) => {
                    Err(std::io::Error::new(std::io::ErrorKind::Other,path.as_str()))
                }
            }
        }
        Err(err) => {
            return Err(std::io::Error::new(err.kind(),path.as_str()));
        }
    }
}

pub fn read_hex_value(path: String) -> Result<u32, std::io::Error>
{
    match fs::read_to_string(&path)
    {
        Ok(content) => {
            match u32::from_str_radix(content.trim(),16)
            {
                Ok(value) => {
                    Ok(value)
                }
                Err(_) => {
                    return Err(std::io::Error::new(std::io::ErrorKind::Other,path.as_str()));
                }
            }

        }

        Err(_err) => {
            return Err(std::io::Error::new(std::io::ErrorKind::Other,path.as_str()));
        }
    }
}

pub fn write_value_as_type<Type: ToString>(path: String, value: Type) -> Result<(),std::io::Error>
{
    if !Path::new(&path).exists()
    {
        return Err(std::io::Error::new(std::io::ErrorKind::Other,format!("File does not exists: {}",path.as_str())));
    }

    match fs::write(path.as_str(), value.to_string().as_str())
    {
        Err(err) => {
            return Err(std::io::Error::new(err.kind(),path.as_str()));
        }

        Ok(_) => {
            Ok(())
        }
    }
}
pub fn int_to_string_power_modes(u32 : u32) -> String {
    match u32 {
        1 => "Quiet".to_string(),
        2 => "Balance".to_string(),
        3 => "Power".to_string(),
        224 => "Extreme".to_string(),
        255 => "Custom".to_string(),
        _ => format!("Unknown({})",u32)
    }
}

pub fn string_power_modes_to_int(mode : &String) -> u32 {
    match mode.as_str() {
        "Quiet"     => 2,
        "Balance"   => 3,
        "Power"     => 4,
        "Extreme"   => 224,
        "Custom"    => 255,
        _ => 0
    }
}