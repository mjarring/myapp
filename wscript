from waflib.Build import BuildContext

APPNAME = "myapp"
VERSION = "0.0.1"


class aarch64_build(BuildContext):
    cmd = "aarch64"
    variant = "aarch64"


class x86_build(BuildContext):
    cmd = "x86"
    variant = "x86"


def options(opt):
    opt.load("compiler_c")
    opt.load("compiler_cxx")


def configure(conf):
    conf.find_program("wayland-scanner", var="WAYLAND_SCANNER")
    base_env = conf.env

    conf.setenv("aarch64", env=base_env)
    conf.env.CC = "aarch64-linux-gnu-gcc"
    conf.env.CXX = "aarch64-linux-gnu-g++"
    conf.env.LIBPATH = ["/usr/lib/aarch64-linux-gnu"]
    conf.load("compiler_c")
    conf.load("compiler_cxx")

    conf.setenv("x86", env=base_env)
    conf.env.CC = "x86_64-linux-gnu-gcc"
    conf.env.CXX = "x86_64-linux-gnu-g++"
    conf.env.AR = "x86_64-linux-gnu-ar"
    conf.env.LIBPATH = ["/usr/lib/x86_64-linux-gnu"]
    conf.load("compiler_c")
    conf.load("compiler_cxx")


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
