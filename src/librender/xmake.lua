mtb_target("mitsuba-render")
    set_kind("shared")
    add_files("*.cpp")
    add_deps("mitsuba-core")

    add_defines("MTS_BUILD_MODULE=MTS_MODULE_RENDER")