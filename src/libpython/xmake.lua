target("mitsuba-python")
    set_kind("shared")
    add_files("*.cpp")
    add_deps("mitsuba-core")
    set_languages("cxx17")

    add_defines("MTS_BUILD_MODULE=MTS_MODULE_PYTHON")

    if is_plat("windows") then
        add_cxxflags("/bigobj")
    else
        add_cxxflags("-fno-strict-aliasing")
    end
