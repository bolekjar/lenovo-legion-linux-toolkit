mod args;
mod sysfs_drivers;
pub mod utils;

use args::Cli;
use clap::Parser;
use crate::sysfs_drivers::others_control::{Attribute, IsACFitForOc, SmartFan, ThermalMode};
use crate::utils::string_power_modes_to_int;

fn main() -> Result<(),std::io::Error> {
   let cli = Cli::parse();

   if cli.smart_fan()
   {
      println!("{}",SmartFan::new()?);
   }

   if cli.win_key()
   {
      println!("Win Key:\n{}",Attribute::new("disable_win_key".to_string())?);
   }

   if cli.touchpad()
   {
      println!("Touch pad:\n{}",Attribute::new("disable_tp".to_string())?);
   }

   if cli.thermal_mode() {
      println!("{}",ThermalMode::new()?);
   }

   if cli.tpd() {
      Attribute::new("disable_tp".to_string())?.set_value(true)?;
   }

   if cli.tpe() {
      Attribute::new("disable_tp".to_string())?.set_value(false)?;
   }

   if cli.wkd() {
      Attribute::new("disable_win_key".to_string())?.set_value(true)?;
   }

   if cli.wke() {
      Attribute::new("disable_win_key".to_string())?.set_value(false)?;
   }

   if cli.is_ac_fit_oc() {
      println!("Is AC fit for OC:\n{}",IsACFitForOc::new()?);
   }

   match cli.smart_fan_set() {
      Some(value) => {
         SmartFan::new()?.set_value(string_power_modes_to_int(value))?;
      }
      None => {}
   }


   match cli.command() {
      Some(cmd) => {
         cmd.parse()?
      }
      None => {}
   }

   Ok(())
}
