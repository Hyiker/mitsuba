target("mitsuba-render")
    set_kind("shared")
    add_files("*.cpp")
    add_deps("mitsuba-core")
    set_languages("cxx17")

    add_defines("MTS_BUILD_MODULE=MTS_MODULE_RENDER")