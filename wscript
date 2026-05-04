APPNAME = "myapp"
VERSION = "0.0.1"


def options(opt):
    opt.load("compiler_c")
    opt.load("compiler_cxx")


def configure(conf):
    conf.load("compiler_c")
    conf.load("compiler_cxx")
    conf.find_program("wayland-scanner", var="WAYLAND_SCANNER")
    conf.check_cfg(
        package="wayland-client",
        args=["--cflags", "--libs"],
        uselib_store="wayland-client",
        mandatory=True,
    )
    conf.check_cfg(
        package="xkbcommon",
        args=["--cflags", "--libs"],
        uselib_store="xkbcommon",
        mandatory=True,
    )


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
        use=["wayland-client", "xkbcommon", "xdg-shell-protocol"],
        lib="rt",
        libpath="/usr/lib/aarch64-linux-gnu",
    )
