mtb_target("mitsuba-core")
    set_kind("shared")
    add_files("*.cpp")

    add_defines("MTS_BUILD_MODULE=MTS_MODULE_CORE")
    if is_plat("windows") then
        add_files("getopt.c")
        add_syslinks("psapi")
    end