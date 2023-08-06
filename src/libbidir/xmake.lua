target("mitsuba-bidir")
    set_kind("shared")
    add_files("*.cpp")
    add_deps("mitsuba-core", "mitsuba-render")
    set_languages("cxx17")

    add_defines("MTS_BUILD_MODULE=MTS_MODULE_BIDIR")
