// SPDX-License-Identifier: GPL-2.0
/*
 * Version of the CLI (--version), the same rule as the application (LenovoLegion/.qmake.conf):
 * LEGION_VERSION from the environment (packaging), else the exact git tag release/X.Y.Z, else 0.0.0
 */
use std::process::Command;

fn git_release_tag() -> Option<String> {
    let output = Command::new("git").args(["describe", "--exact-match", "--tags"]).output().ok()?;

    if !output.status.success() {
        return None;
    }

    String::from_utf8(output.stdout).ok()?.trim().strip_prefix("release/").map(str::to_string)
}

fn main() {
    let version = std::env::var("LEGION_VERSION").ok()
        .filter(|version| !version.is_empty())
        .or_else(git_release_tag)
        .unwrap_or_else(|| "0.0.0".to_string());

    println!("cargo:rustc-env=LEGION_VERSION={}", version);
    println!("cargo:rerun-if-env-changed=LEGION_VERSION");
    println!("cargo:rerun-if-changed=../.git/HEAD");
    println!("cargo:rerun-if-changed=../.git/refs/tags");
}
