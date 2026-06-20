# Project Structure Outline: myapp

This document outlines the directory structure, components, and build architecture of **myapp**, a Wayland-based application targeting the Raspberry Pi (aarch64) and local x86 environments.

---

## Workspace Directory Tree

Below is the high-level layout of the workspace:

```text
/home/arrington/myapp/
├── Dockerfile                  # Container definition for the compilation environment
├── README.md                   # Build and development instructions
├── deploy-rpi.sh               # Deployment script to copy the binary to a Raspberry Pi
├── ssh.sh                      # Helper script to SSH into the Raspberry Pi
├── waf                         # Wrapper script executing Waf commands (Dockerized or host)
├── wscript                     # Waf build configuration script
├── source/                     # C++ source code directory
│   └── main.cpp                # Main application source code
└── submodules/                 # Git submodules
    └── waf/                    # Waf build system source tree
```

---

## Detailed Directory & File Breakdown

### 1. Build & Tooling Scripts

* **[waf](file:///home/arrington/myapp/waf)**: A Python script wrapper configured to run with `uv`.
  * **Docker Mode (Default)**: Automatically builds the `myapp-builder` container image using the local `Dockerfile` and executes the commands inside it.
  * **Host Mode**: When passed the `--without-docker` argument, it runs the `waf-light` binary directly on the host system, skipping all Docker calls.
* **[wscript](file:///home/arrington/myapp/wscript)**: The configuration file defining rules, targets, and compiler settings for the Waf build system. It manages:
  * Native build configuration using the host system compilers.
  * Generating and symlinking `compile_commands.json` (via the helper command `clangd`) for code intelligence tools.
  * Generating Wayland protocol client header and protocol code via `wayland-scanner`.

### 2. Environment & Deployment Configuration

* **[Dockerfile](file:///home/arrington/myapp/Dockerfile)**: Defines a Debian-based container (`myapp-builder`) pre-installed with cross-compilation toolchains for ARM64 (`aarch64-linux-gnu`) and x86 (`x86_64-linux-gnu`), Wayland library development headers (`libwayland-dev`), `libxkbcommon-dev`, and Astral's `uv` tool.
* **[deploy-rpi.sh](file:///home/arrington/myapp/deploy-rpi.sh)**: Automates transferring the compiled binary from `build-rpi/myapp` to the target Raspberry Pi over SSH/SCP. Defaults to `192.168.50.11` but can be overridden with the `--host` argument.
* **[ssh.sh](file:///home/arrington/myapp/ssh.sh)**: A simple convenience script to log in via SSH to the default Raspberry Pi IP `192.168.50.11` as `root`.

### 3. Application Code & Submodules

* **[source/](file:///home/arrington/myapp/source)**:
  * **[main.cpp](file:///home/arrington/myapp/source/main.cpp)**: The core C++ codebase containing the Wayland client window setup, event handlers, and application logic.
* **[submodules/](file:///home/arrington/myapp/submodules)**:
  * **[waf/](file:///home/arrington/myapp/submodules/waf)**: Contains the unmodified upstream repository for Waf, including the `waf-light` binary and the `waflib` libraries.
