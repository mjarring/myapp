# myapp

Wayland app for raspberry pi

## Building

The project uses `waf` for building. All builds run inside a Docker container automatically.

### Build Dependencies

Install the following packages on the build host:

```bash
wayland-devel
wayland-protocols-devel
libxkbcommon-devel
```

### Build for aarch64 (Raspberry Pi)

```bash
./waf aarch64
```

### Build for x86

```bash
./waf x86
```

## clangd

To enable code completion and navigation via clangd,
you need a `compile_commands.json` file in the project root.

Because the build runs inside a Docker container, standard compilation database
generation produces internal paths (e.g., `/work/...`) that do not exist on your
host machine. We provide helper commands to generate the database and fix these
paths for your local environment.

### Setup for aarch64

```bash
./waf aarch64_clangd
```

### Setup for x86

```bash
./waf x86_clangd
```

These commands perform the following:

1. **Generate**: Creates the `compile_commands.json` for the specific architecture.
2. **Fix Paths**: Automatically replaces the internal container path `/work` with
your actual project path on the host.
3. **Symlink**: Creates a `compile_commands.json` symbolic link in the project
root pointing to the generated file.

### Manual Database Generation

If you only want to generate the compilation database in the build directory
without symlinking or path fixing, use the `clangdb` variants:

- `./waf aarch64_clangdb`
- `./waf x86_clangdb`
