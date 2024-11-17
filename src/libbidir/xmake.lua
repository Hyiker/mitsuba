mtb_target("mitsuba-bidir")
    set_kind("shared")
    add_files("*.cpp")
    add_deps("mitsuba-core", "mitsuba-render")

    add_defines("MTS_BUILD_MODULE=MTS_MODULE_BIDIR")
