from waflib import Context, Scripting
import os

APPNAME = "myapp"
VERSION = "0.0.1"
WAF_TOOL_DIR = "submodules/waf/waflib/extras"


def options(opt):
    opt.load("compiler_c")
    opt.load("compiler_cxx")
    opt.load("clang_compilation_database", tooldir=[f"{WAF_TOOL_DIR}"])


def configure(conf):
    conf.find_program("wayland-scanner", var="WAYLAND_SCANNER")
    conf.load("compiler_c")
    conf.load("compiler_cxx")
    conf.load("clang_compilation_database", tooldir=[f"{WAF_TOOL_DIR}"])

    # Set debug flags by default
    conf.env.append_unique("CFLAGS", ["-g", "-O0"])
    conf.env.append_unique("CXXFLAGS", ["-g", "-O0"])


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
        source=["source/main.cpp"],
        target=APPNAME,
        includes=".",
        use=["xdg-shell-protocol"],
        lib=["wayland-client", "xkbcommon", "rt"],
    )


def clangd(ctx):
    """tells clangd to use the compile_commands.json in the build output"""
    Scripting.run_command("clangdb")
    src = os.path.join(Context.out_dir, "compile_commands.json")
    dst = "compile_commands.json"
    if os.path.exists(dst) or os.path.islink(dst):
        os.remove(dst)
    rel_src = os.path.relpath(src, os.path.dirname(os.path.abspath(dst)))
    os.symlink(rel_src, dst)
    print(f"Done! clangd now uses {src} (symlinked as {rel_src})")
