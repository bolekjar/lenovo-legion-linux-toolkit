#![allow(dead_code)]

use std::collections::BTreeSet;
use std::fmt::{Display, Formatter};
use std::fs;
use std::path::Path;
use crate::utils::{*};


static SYSFS_SYSTEM_CPU: &str = "/sys/devices/system/cpu/";
static SYSFS_CPU_ATOM: &str = "/sys/devices/cpu_atom/";
static SYSFS_CPU_CORES: &str = "/sys/devices/cpu_core/";


#[derive(Debug,Default,Clone)]
pub struct Topology {
    kernel_max: u32,

    offline: BTreeSet<u32>,
    online: BTreeSet<u32>,
    possible: BTreeSet<u32>,
    present: BTreeSet<u32>,
    atom: BTreeSet<u32>,
    core: BTreeSet<u32>
}

#[derive(Debug,Default,Clone)]
pub struct CPUFreq
{
    path: String,


    affected_cpus: BTreeSet<u32>,
    base_frequency: u32,
    cpuinfo_avg_freq: u32,
    cpuinfo_max_freq: u32,
    cpuinfo_min_freq: u32,
    cpuinfo_transition_latency: u32,
    energy_performance_available_preferences: BTreeSet<String>,
    energy_performance_preference: String,
    related_cpus: BTreeSet<u32>,
    scaling_available_governors: BTreeSet<String>,
    scaling_governor: String,
    scaling_cur_freq: u32,
    scaling_max_freq: u32,
    scaling_min_freq: u32,
    scaling_driver: String,
    scaling_setspeed: String,
}

#[derive(Debug,Default,Clone)]
pub struct SMT
{
    path: String,

    active: u8,
    control: String,
}

 #[derive(Debug,Default,Clone)]
 pub struct CPUXTopology
 {
     path: String,

     cluster_cpus: u32,
     cluster_cpus_list: BTreeSet<u32>,
     cluster_id: u32,
     core_cpus: u32,
     core_cpus_list: BTreeSet<u32>,
     core_id: u32,
     core_siblings: u32,
     core_siblings_list: BTreeSet<u32>,
     die_cpus: u32,
     die_cpus_list: BTreeSet<u32>,
     die_id: u32,
     package_cpus: u32,
     package_cpus_list: BTreeSet<u32>,
     physical_package_id: u32,
     thread_siblings: u32,
     thread_siblings_list: BTreeSet<u32>
 }

#[derive(Debug,Default,Clone)]
pub struct CPUX
{
    path: String,
    id: u32,

    cpu_freq: Option<CPUFreq>,
    topology: Option<CPUXTopology>,
    online: Option<u8>,
}

 #[derive(Debug,Default,Clone)]
 pub struct CPUs
 {
     cpus: Vec<CPUX>,
     smt: SMT
 }

pub struct CpusIter {
    cpus_slice: Vec<u32>,
}

impl Iterator for CpusIter {
    type Item = u32;
    fn next(&mut self) -> Option<Self::Item> {
        self.cpus_slice.pop()
    }
}

pub struct StringValuesIter {
    values : Vec<String>
}

impl Iterator for StringValuesIter {
    type Item = String;
    fn next(&mut self) -> Option<Self::Item> {
        self.values.pop()
    }
}

pub struct CPUXIter<'a> {
    values : Vec<&'a CPUX>
}

impl<'a> Iterator for CPUXIter<'a> {
    type Item = &'a CPUX;

    fn next(&mut self) -> Option<Self::Item> {
        self.values.pop()
    }
}

pub fn parse_cpu_range(path : &str ) -> Result<BTreeSet<u32>,std::io::Error>
{
    match fs::read_to_string(Path::new(path)) {
        Ok(contents) => {
            return parse_cpu_range_value(contents.as_str());
        }
        Err(err) => {
            return Err(std::io::Error::new(err.kind(), format!("Reading of file {} error: {}",path,err.to_string())));
        }
    }
}

impl Topology {

    pub fn new() -> Result<Topology,std::io::Error> {
        let mut topology = Default::default();

        Self::refresh(&mut topology)?;

        Ok(topology)
    }

    pub fn refresh(&mut self) -> Result<&Topology, std::io::Error>
    {
        self.kernel_max = read_value_as_type::<u32>(SYSFS_SYSTEM_CPU.to_string() + "kernel_max" )?;

        for s  in {
            let vec : [(String,fn(topology: & mut Topology,cpu_range: BTreeSet<u32>));_] =
                [
                    ({SYSFS_SYSTEM_CPU.to_string() + "offline" }, |topology: & mut Topology,cpu_range: BTreeSet<u32>| { topology.offline = cpu_range}),
                    ({SYSFS_SYSTEM_CPU.to_string() + "online" },  |topology: & mut Topology,cpu_range: BTreeSet<u32>| { topology.online = cpu_range}),
                    ({SYSFS_SYSTEM_CPU.to_string() + "possible" },|topology: & mut Topology,cpu_range: BTreeSet<u32>| { topology.possible = cpu_range}),
                    ({SYSFS_SYSTEM_CPU.to_string() + "present" }, |topology: & mut Topology,cpu_range: BTreeSet<u32>| { topology.present = cpu_range}),
                    ({SYSFS_CPU_ATOM.to_string() + "cpus" }, |topology: & mut Topology,cpu_range: BTreeSet<u32>| { topology.atom = cpu_range}),
                    ({SYSFS_CPU_CORES.to_string() + "cpus" }, |topology: & mut Topology,cpu_range: BTreeSet<u32>| { topology.core = cpu_range}),
                ];
            vec
        }.iter()
        {
            s.1(self, parse_cpu_range(s.0.as_str())?);
        }

        Ok(self)
    }

    pub fn iter_offline(&self) -> CpusIter {
        CpusIter {
            cpus_slice: self.offline.iter().cloned().collect(),
        }
    }

    pub fn iter_online(&self) -> CpusIter {
        CpusIter {
            cpus_slice: self.online.iter().cloned().collect(),
        }
    }

    pub fn iter_possible(&self) -> CpusIter {
        CpusIter {
            cpus_slice: self.possible.iter().cloned().collect(),
        }
    }

    pub fn iter_present(&self) -> CpusIter {
        CpusIter {
            cpus_slice: self.present.iter().cloned().collect(),
        }
    }

    pub fn iter_atom(&self) -> CpusIter {
        CpusIter {
            cpus_slice: self.atom.iter().cloned().collect(),
        }
    }

    pub fn iter_core(&self) -> CpusIter {
        CpusIter {
            cpus_slice: self.core.iter().cloned().collect(),
        }
    }

    pub fn offline(&self) -> &BTreeSet<u32>
    {
        &self.offline
    }

    pub fn online(&self) -> &BTreeSet<u32>
    {
        &self.online
    }

    pub fn possible(&self) -> &BTreeSet<u32>
    {
        &self.possible
    }

    pub fn present(&self) -> &BTreeSet<u32>
    {
        &self.present
    }

    pub fn atom(&self) -> &BTreeSet<u32>
    {
        &self.atom
    }

    pub fn core(&self) -> &BTreeSet<u32>
    {
        &self.core
    }

    pub fn kernel_max(&self) -> u32 {
        self.kernel_max
    }
}

impl CPUFreq {
    pub fn new(cpu : u32) -> Result<CPUFreq,std::io::Error> {
        let mut cpufreq = CPUFreq {
            path: SYSFS_SYSTEM_CPU.to_string() + format!("cpu{}/cpufreq/", cpu).as_str(),
            ..Self::default()
        };

        if !Path::new(&cpufreq.path).exists() {
            return Err(std::io::Error::new(std::io::ErrorKind::NotFound,cpufreq.path.as_str()));
        }

        CPUFreq::refresh(& mut cpufreq)?;

        return Ok(cpufreq);
    }


    pub fn refresh(&mut self) -> Result<&CPUFreq, std::io::Error> {

        self.affected_cpus = parse_cpu_range({self.path.clone() + "affected_cpus"}.as_str())?;
        self.base_frequency = read_value_as_type::<u32>(self.path.clone() + "base_frequency" )?;
        self.cpuinfo_avg_freq = read_value_as_type::<u32>(self.path.clone() + "cpuinfo_avg_freq" )?;
        self.cpuinfo_min_freq = read_value_as_type::<u32>(self.path.clone() + "cpuinfo_min_freq" )?;
        self.cpuinfo_max_freq = read_value_as_type::<u32>(self.path.clone() + "cpuinfo_max_freq" )?;
        self.cpuinfo_transition_latency = read_value_as_type::<u32>(self.path.clone() + "cpuinfo_transition_latency" )?;
        self.energy_performance_available_preferences = read_value_as_type::<String>(self.path.clone() + "energy_performance_available_preferences" )?.
            split(" ").map(|s|{s.trim().to_string()}).collect();
        self.energy_performance_preference = read_value_as_type::<String>(self.path.clone() + "energy_performance_preference" )?.trim().to_string();
        self.related_cpus = parse_cpu_range({self.path.clone() + "related_cpus"}.as_str())?;
        self.scaling_available_governors = read_value_as_type::<String>(self.path.clone() + "scaling_available_governors" )?.
            split(" ").map(|s|{s.trim().to_string()}).collect();
        self.scaling_governor = read_value_as_type::<String>(self.path.clone() + "scaling_governor" )?.trim().to_string();
        self.scaling_cur_freq = read_value_as_type::<u32>(self.path.clone() + "scaling_cur_freq" )?;
        self.scaling_max_freq = read_value_as_type::<u32>(self.path.clone() + "scaling_max_freq" )?;
        self.scaling_min_freq = read_value_as_type::<u32>(self.path.clone() + "scaling_min_freq" )?;
        self.scaling_driver = read_value_as_type::<String>(self.path.clone() + "scaling_driver" )?.trim().to_string();
        self.scaling_setspeed = read_value_as_type::<String>(self.path.clone() + "cpuinfo_max_freq" )?.trim().to_string();

        Ok(self)
    }

    pub fn iter_affected_cpus(&self) -> CpusIter {
        CpusIter {
            cpus_slice: self.affected_cpus.iter().cloned().collect(),
        }
    }

    pub fn iter_related_cpus(&self) -> CpusIter {
        CpusIter {
            cpus_slice: self.related_cpus.iter().cloned().collect(),
        }
    }

    pub fn iter_performance_available_preferences(&self) -> StringValuesIter {
        StringValuesIter {
            values: self.energy_performance_available_preferences.iter().cloned().collect(),
        }
    }

    pub fn iter_performance_available_governors(&self) -> StringValuesIter {
        StringValuesIter {
            values: self.scaling_available_governors.iter().cloned().collect(),
        }
    }

    pub fn base_frequency(&self)  -> u32 { self.base_frequency }
    pub fn cpuinfo_avg_freq (&self) -> u32 { self.cpuinfo_avg_freq }
    pub fn cpuinfo_min_freq (&self) -> u32 { self.cpuinfo_min_freq }
    pub fn cpuinfo_max_freq (&self) -> u32 { self.cpuinfo_max_freq }
    pub fn cpuinfo_transition_latency(&self) -> u32 { self.cpuinfo_transition_latency }
    pub fn energy_performance_preference (&self) -> &String{ &self.energy_performance_preference }
    pub fn scaling_governor(&self) -> &String{ &self.scaling_governor }
    pub fn scaling_cur_freq (&self) -> u32 { self.scaling_cur_freq }
    pub fn scaling_max_freq (&self) -> u32 { self.scaling_max_freq }
    pub fn scaling_min_freq (&self) -> u32 { self.scaling_min_freq }
    pub fn scaling_driver (&self) -> &String{ &self.scaling_driver }
    pub fn scaling_setspeed (&self) -> &String{ &self.scaling_setspeed }

    pub fn freq_min (&self, min: u32) -> Result<&CPUFreq,std::io::Error> {
        match write_value_as_type::<u32>(self.path.clone() + "scaling_min_freq",min)
        {
            Ok(_v) => Ok(self),
            Err(e) => Err(e),
        }
    }

    pub fn freq_max(&self, max: u32) -> Result<&CPUFreq,std::io::Error>
    {
        match write_value_as_type::<u32>(self.path.clone() + "scaling_max_freq",max)
        {
            Ok(_v) => Ok(self),
            Err(e) => Err(e),
        }
    }

    pub fn governor(&self,governor: &String ) -> Result<&CPUFreq,std::io::Error>
    {
        match write_value_as_type::<String>(self.path.clone() + "scaling_governor",governor.clone())
        {
            Ok(_v) => Ok(self),
            Err(e) => Err(e),
        }
    }
}


impl CPUX {
    pub fn new(cpu : u32) -> Result<CPUX,std::io::Error> {
        let mut cpux  = CPUX {
            path: SYSFS_SYSTEM_CPU.to_string() + format!("cpu{}/", cpu).as_str(),
            id: cpu,
            ..Self::default()
        };

        CPUX::refresh(&mut cpux)?;

        Ok(cpux)
    }

    pub fn refresh (&mut self) -> Result<&CPUX, std::io::Error> {
        let path = self.path.clone() + "online";

        self.online = None;
        self.cpu_freq = None;
        self.topology = None;

        if Path::new(&path).exists() {
            self.online = Some(read_value_as_type::<u8>(path)?)
        }

        match self.online {
            None => {
                self.cpu_freq = Some(CPUFreq::new(self.id)?);
                self.topology = Some(CPUXTopology::new(self.id)?)
            }
            Some(value) => {
               if value != 0
               {
                   self.cpu_freq = Some(CPUFreq::new(self.id)?);
                   self.topology = Some(CPUXTopology::new(self.id)?)
               }
            }
        }

        Ok(self)
    }
    pub fn cpufreq(&self) -> &Option<CPUFreq> {
        &self.cpu_freq
    }

    pub fn topology(&self) -> &Option<CPUXTopology> {
        &self.topology
    }

    pub fn online(&self) -> Option<u8> {
        self.online
    }

    pub fn enable(&self)  -> Result<&CPUX, std::io::Error> {
        let path = self.path.clone() + "online";
        write_value_as_type::<bool>(path,true)?;
        Ok(self )
    }

    pub fn disable(&self) -> Result<&CPUX, std::io::Error> {
        let path = self.path.clone() + "online";
        write_value_as_type::<bool>(path,false)?;
        Ok(self )
    }

    pub fn freq_min(&self, min: u32) -> Result<&CPUX, std::io::Error> {
        match &self.cpu_freq {
            Some(value ) => {
                match value.freq_min(min)
                {
                    Ok (_) => {
                        Ok(self)
                    }
                    Err(err ) => {
                        Err(std::io::Error::new(err.kind(),err.to_string()))
                    }
                }
            }
            None => {Ok(self)}
        }
    }

    pub fn freq_max(&self, max: u32) -> Result<&CPUX, std::io::Error> {
        match &self.cpu_freq {
            Some(value) => {
                match value.freq_max(max)
                {
                    Ok (_) => {
                        Ok(self)
                    }
                    Err(err ) => {
                        Err(std::io::Error::new(err.kind(),err.to_string()))
                    }
                }
            }
            None => {
                Ok(self)
            }
        }
    }

    pub fn governor(&self, governor: &String) -> Result<&CPUX, std::io::Error> {
        match &self.cpu_freq {
            Some(value ) => {
                match value.governor(governor)
                {
                    Ok(_) => {
                        Ok(self)
                    }
                    Err(err) => {
                        Err(std::io::Error::new(err.kind(),err.to_string()))
                    }
                }
            }
            None => {
                Ok(self)
            }
        }
    }
}

impl SMT {
    pub fn new () -> Result<SMT,std::io::Error> {

        let mut smt = SMT {
            path: SYSFS_SYSTEM_CPU.to_string() + "smt/",
            ..Self::default()
        };

        SMT::refresh(&mut smt)?;
        Ok(smt)
    }

    pub fn active(&self) -> u8 {
        self.active
    }

    pub fn control(&self) -> &String {
        &self.control
    }

    pub fn refresh(& mut self)-> Result<&SMT,std::io::Error> {
        self.active = read_value_as_type::<u8>(self.path.clone() + "active")?;
        self.control = read_value_as_type::<String>(self.path.clone() + "control")?;
        Ok(self)
    }
}


 impl CPUs {
     pub fn new () -> Result<CPUs,std::io::Error> {
         let mut cpus = CPUs{
           cpus : Vec::new(),
           smt: SMT::new()?
         };

         for cpu in  Topology::new()?.iter_possible()
         {
             if (cpu as usize) > cpus.cpus.len()
             {
                 cpus.cpus.resize(cpu as usize + 1,CPUX::default());
             }

             cpus.cpus[cpu as usize] = CPUX::new(cpu)?;
         }

         Ok(cpus)
     }

     pub fn refresh (&mut self) -> Result<&CPUs,std::io::Error> {
         for cpu in self.cpus.iter_mut()
         {
             cpu.refresh()?;
         }
         Result::Ok(self)
     }

     pub fn iter (&self) -> CPUXIter<'_> {
         CPUXIter {
             values: self.cpus.iter().collect(),
         }
     }

     pub fn mut_iter (&mut self) -> impl Iterator<Item = &mut CPUX> {
         self.cpus.iter_mut()
     }

 }


 impl CPUXTopology {
     pub fn new (cpu : u32) -> Result<CPUXTopology,std::io::Error> {
         let mut cpux_topology = CPUXTopology {
             path: SYSFS_SYSTEM_CPU.to_string() + format!("cpu{}/topology/", cpu).as_str(),
             ..Self::default()
         };

         CPUXTopology::refresh(&mut cpux_topology)?;

        Ok(cpux_topology)
     }

     pub fn refresh (&mut self) -> Result<&CPUXTopology,std::io::Error> {

         self.cluster_cpus = read_hex_value(self.path.clone() + "cluster_cpus")?;
         self.cluster_cpus_list= parse_cpu_range({self.path.clone() + "cluster_cpus_list"}.as_str())?;
         self.cluster_id = read_value_as_type::<u32>(self.path.clone() + "cluster_id")?;
         self.core_cpus = read_hex_value(self.path.clone() + "core_cpus")?;
         self.core_cpus_list= parse_cpu_range({self.path.clone() + "core_cpus_list"}.as_str())?;
         self.core_id = read_value_as_type::<u32>(self.path.clone() + "core_id")?;
         self.core_siblings = read_hex_value(self.path.clone() + "core_siblings")?;
         self.core_siblings_list = parse_cpu_range({self.path.clone() + "core_siblings_list"}.as_str())?;
         self.die_cpus = read_hex_value(self.path.clone() + "die_cpus")?;
         self.die_cpus_list = parse_cpu_range({self.path.clone() + "die_cpus_list"}.as_str())?;
         self.die_id= read_value_as_type::<u32>(self.path.clone() + "die_id")?;
         self.package_cpus= read_hex_value(self.path.clone() + "package_cpus")?;
         self.package_cpus_list= parse_cpu_range({self.path.clone() + "package_cpus_list"}.as_str())?;
         self.physical_package_id= read_value_as_type::<u32>(self.path.clone() + "physical_package_id")?;
         self.thread_siblings= read_hex_value(self.path.clone() + "thread_siblings")?;
         self.thread_siblings_list= parse_cpu_range({self.path.clone() + "thread_siblings_list"}.as_str())?;

         Ok(self)
     }

     pub fn cluster_cpus (&self) -> u32 {
         self.cluster_cpus
     }

     pub fn cluster_cpus_list_iter (&self) -> CpusIter {
         CpusIter {
             cpus_slice: self.cluster_cpus_list.iter().cloned().collect(),
         }
     }

     pub fn core_cpus (&self) -> u32 {
         self.core_cpus
     }

     pub fn core_cpus_list_iter (&self) -> CpusIter {
         CpusIter {
             cpus_slice: self.core_cpus_list.iter().cloned().collect(),
         }
     }

     pub fn cluster_id (&self) -> u32 {
         self.cluster_id
     }

     pub fn core_siblings (&self) -> u32 {
         self.core_siblings
     }

     pub fn core_siblings_list_iter (&self) -> CpusIter {
         CpusIter {
             cpus_slice: self.core_siblings_list.iter().cloned().collect(),
         }
     }

     pub fn core_id ( &self ) -> u32 {
         self.core_id
     }

     pub fn package_cpus (&self) -> u32 {
         self.package_cpus
     }

     pub fn package_cpus_list_iter (&self) -> CpusIter {
         CpusIter {
             cpus_slice: self.package_cpus_list.iter().cloned().collect(),
         }
     }

     pub fn physical_package_id (&self) -> u32 {
         self.physical_package_id
     }

     pub fn thread_siblings (&self) -> u32 {
         self.thread_siblings
     }

     pub fn thread_siblings_list_iter (&self) -> CpusIter {
         CpusIter {
             cpus_slice: self.thread_siblings_list.iter().cloned().collect(),
         }
     }

     pub fn die_cpus (&self) -> u32 {
         self.die_cpus
     }

     pub fn die_cpus_list_iter (&self) -> CpusIter {
         CpusIter {
             cpus_slice: self.die_cpus_list.iter().cloned().collect(),
         }
     }

     pub fn die_id ( &self ) -> u32 {
         self.die_id
     }

 }


 impl Display for CPUs {
     fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
         _ = write!(f, "cpus: [\n");
         for cpu in self.cpus.iter() {
              _ =  write!(f, "{}", cpu);
         }
         _ = write!(f, "{}\n", self.smt);
         _ = write!(f, "]\n");
         Ok(())
     }
 }

 impl Display for CPUX {
     fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
          _ = write!(f, "cpu{}: [\n",self.id);
          _ = write!(f, "{}\n",{
              match &self.cpu_freq {
                  Some(value) => value.to_string(),
                  None => "cpufreq: N/A".to_string()
              }
           } );
          _ = write!(f, "{}\n", {
             match &self.topology {
                 Some(value) => value.to_string(),
                 None => "topology: N/A".to_string()
            }
         });

         match self.online
         {
             Some(online) => {
                 let _ =  write!(f, "online (W): {}\n", online);
             }
             None => {}
         };

         _ = write!(f, "]\n");
         Ok(())
     }
 }

 impl Display for CPUFreq {
     fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
         _ = write!(f,"cpufreq: [\n");
         _ = write!(f, "\taffected_cpus: {}\n", self.affected_cpus.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
         _ = write!(f, "\tbase_frequency: {} Hz\n", self.base_frequency);
         _ = write!(f, "\tcpuinfo_avg_freq: {} Hz\n", self.cpuinfo_avg_freq);
         _ = write!(f, "\tcpuinfo_min_freq: {} Hz\n", self.cpuinfo_min_freq);
         _ = write!(f, "\tcpuinfo_max_freq: {} Hz\n", self.cpuinfo_max_freq);
         _ = write!(f, "\tscaling_cur_freq: {} Hz\n", self.scaling_cur_freq);
         _ = write!(f, "\tscaling_min_freq (W): {} Hz\n", self.scaling_min_freq);
         _ = write!(f, "\tscaling_max_freq (W): {} Hz\n", self.scaling_max_freq);
         _ = write!(f, "\tscaling_available_governors: {}\n", self.scaling_available_governors.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
         _ = write!(f, "\tscaling_governor (W): {}\n",self.scaling_governor);
         _ = write!(f, "\tcpuinfo_transition_latency: {}\n", self.cpuinfo_transition_latency);
         _ = write!(f, "\tenergy_performance_available_preferences: {}\n",self.energy_performance_available_preferences.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
         _ = write!(f, "\tenergy_performance_preference: {}\n",self.energy_performance_preference);
         _ = write!(f, "\trelated_cpus: {}\n",self.related_cpus.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
         _ = write!(f, "\tscaling_driver: {}\n", self.scaling_driver);
         _ = write!(f, "\tscaling_setspeed: {} Hz\n", self.scaling_setspeed);
         _ = write!(f,"]");
         Ok(())
     }
 }

 impl Display for CPUXTopology {
     fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
         _ = write!(f, "topology: [\n");
         _ = write!(f, "\tcluster_cpus: {:x}\n",self.cluster_cpus);
         _ = write!(f, "\tcluster_cpus_list: {}\n",self.cluster_cpus_list.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
         _ = write!(f, "\tcluster_id: {:x}\n",self.cluster_id);
         _ = write!(f, "\tcore_cpus: {:x}\n",self.core_cpus);
         _ = write!(f, "\tcore_cpus_list: {}\n",self.core_cpus_list.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
         _ = write!(f, "\tcore_id: {}\n",self.core_id);
         _ = write!(f, "\tcore_siblings: {:x}\n",self.core_siblings);
         _ = write!(f, "\tcore_siblings_list: {}\n",self.core_siblings_list.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
         _ = write!(f, "\tdie_cpus: {:x}\n",self.die_cpus);
         _ = write!(f, "\tdie_cpus_list: {}\n",self.die_cpus_list.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
         _ = write!(f, "\tdie_id: {}\n",self.die_id);
         _ = write!(f, "\tpackage_cpus_list: {}\n",self.package_cpus_list.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
         _ = write!(f, "\tpackage_cpus: {:x}\n",self.package_cpus);
         _ = write!(f, "\tphysical_package_id: {}\n",self.physical_package_id);
         _ = write!(f, "\tthread_siblings: {:x}\n",self.thread_siblings);
         _ = write!(f, "\tthread_siblings_list: {}\n", self.thread_siblings_list.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
         _ = write!(f, "]");

         Ok(())
     }
 }


impl Display for SMT {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        _ = write!(f, "smt: [\n");
        _ = write!(f, "\tactive: {}\n",self.active);
        _ = write!(f, "\tcontrol (W): {}\n",self.control);
        _ = write!(f, "]");

        Ok(())
    }
}

impl Display for Topology {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        _ = write!(f, "topology: [\n");
        _ = write!(f, "\tkernel_max: {}\n",self.kernel_max);
        _ = write!(f, "\toffline: {}\n",self.offline.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
        _ = write!(f, "\tonline: {}\n",self.online.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
        _ = write!(f, "\tpossible: {}\n",self.possible.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
        _ = write!(f, "\tpresent: {}\n",self.present.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
        _ = write!(f, "\tatom: {}\n",self.atom.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
        _ = write!(f, "\tcore: {}\n",self.core.iter().map(|i| i.to_string() ).collect::<Vec<String>>().join(","));
        _ = write!(f, "]");
        Ok(())
    }

}