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

### Build

```bash
./waf build
```

## clangd

To enable code completion and navigation via clangd,
you need a `compile_commands.json` file in the project root.

We provide a helper command to generate the database:

```bash
./waf clangd
```

This command performs the following:

1. **Generate**: Creates the `compile_commands.json`.
2. **Symlink**: Creates a `compile_commands.json` symbolic link in the project root pointing to the generated file.

### Manual Database Generation

If you only want to generate the compilation database in the build directory without symlinking:

```bash
./waf clangdb
```
