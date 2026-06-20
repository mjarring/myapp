from waflib import Context, Scripting
import os
from waflib.Build import BuildContext
from waflib.extras.clang_compilation_database import ClangDbContext

APPNAME = "myapp"
VERSION = "0.0.1"
WAF_TOOL_DIR = "submodules/waf/waflib/extras"


class aarch64_build(BuildContext):
    cmd = "aarch64"
    variant = "aarch64"


class x86_build(BuildContext):
    cmd = "x86"
    variant = "x86"


class aarch64_clangdb(ClangDbContext):
    cmd = "aarch64_clangdb"
    variant = "aarch64"


class x86_clangdb(ClangDbContext):
    cmd = "x86_clangdb"
    variant = "x86"


def _setup_clangd(ctx, variant):
    Scripting.run_command(f"{variant}_clangdb")

    # The compilation database is written to the build directory on the host
    src = os.path.join(Context.out_dir, variant, "compile_commands.json")
    dst = "compile_commands.json"

    # Post-process compile_commands.json to fix paths from /work to host path
    # Note: This runs on the host, so we can use os.path.abspath
    if os.path.exists(src):
        with open(src, "r") as f:
            content = f.read()

        # Replace /work with the actual absolute path of the project on the host
        # The host path is passed via the HOST_PROJECT_PATH environment variable
        host_path = os.environ.get("HOST_PROJECT_PATH", "/work")
        fixed_content = content.replace("/work", host_path)

        with open(src, "w") as f:
            f.write(fixed_content)
        print(f"Fixed paths in {src} using host path {host_path}")

    if os.path.exists(dst) or os.path.islink(dst):
        os.remove(dst)

    # Use relative path for the symlink
    rel_src = os.path.relpath(src, os.path.dirname(os.path.abspath(dst)))
    os.symlink(rel_src, dst)
    print(f"Done! clangd now uses {src} (symlinked as {rel_src})")


def aarch64_clangd(ctx):
    """tells clangd to use the compile_commands.json in the build output for aarch64"""
    _setup_clangd(ctx, "aarch64")


def x86_clangd(ctx):
    """tells clangd to use the compile_commands.json in the build output for x86"""
    _setup_clangd(ctx, "x86")


def options(opt):
    opt.load("compiler_c")
    opt.load("compiler_cxx")
    opt.load("clang_compilation_database", tooldir=[f"{WAF_TOOL_DIR}"])


def configure(conf):
    conf.find_program("wayland-scanner", var="WAYLAND_SCANNER")
    base_env = conf.env

    conf.setenv("aarch64", env=base_env)
    conf.env.CC = "aarch64-linux-gnu-gcc"
    conf.env.CXX = "aarch64-linux-gnu-g++"
    conf.env.LIBPATH = ["/usr/lib/aarch64-linux-gnu"]
    conf.load("compiler_c")
    conf.load("compiler_cxx")
    conf.load("clang_compilation_database", tooldir=[f"{WAF_TOOL_DIR}"])

    conf.setenv("x86", env=base_env)
    conf.env.CC = "x86_64-linux-gnu-gcc"
    conf.env.CXX = "x86_64-linux-gnu-g++"
    conf.env.AR = "x86_64-linux-gnu-ar"
    conf.env.LIBPATH = ["/usr/lib/x86_64-linux-gnu"]
    conf.load("compiler_c")
    conf.load("compiler_cxx")
    conf.load("clang_compilation_database", tooldir=[f"{WAF_TOOL_DIR}"])


def build(bld):
    wayland_xml_path = "/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml"
    bld(
        rule="${WAYLAND_SCANNER} client-header ${SRC} ${TGT}",
        source=bld.root.find_resource(wayland_xml_path),
        target="xdg-shell-client-protocol.h",
    )
    bld(
        rule="${WAYLAND_SCANNER} private-code ${SRC} ${TGT}",
        source=bld.root.find_resource(wayland_xml_path),
        target="xdg-shell-protocol.c",
    )
    bld.objects(
        features="c",
        source="xdg-shell-protocol.c",
        target="xdg-shell-protocol",
        includes=".",
    )
    bld.program(
        features="cxx cxxprogram",
        source=["source/myapp.cpp"],
        target=APPNAME,
        includes=".",
        use=["xdg-shell-protocol"],
        lib=["wayland-client", "xkbcommon", "rt"],
        libpath=bld.env.LIBPATH,
    )
