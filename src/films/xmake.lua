add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")

mtb_plugin_target("ldrfilm")
    add_files("ldrfilm.cpp")

mtb_plugin_target("hdrfilm")
    add_files("hdrfilm.cpp")

mtb_plugin_target("mfilm")
    add_files("mfilm.cpp", "cnpy.cpp")

mtb_plugin_target("tiledhdrfilm")
    add_files("tiledhdrfilm.cpp")