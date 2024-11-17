add_deps("mitsuba-render")

mtb_plugin_target("addimages")
    add_files("addimages.cpp")

mtb_plugin_target("joinrgb")
    add_files("joinrgb.cpp")

mtb_plugin_target("cylclip")
    add_deps("mitsuba-hw")
    add_files("cylclip.cpp")

mtb_plugin_target("kdbench")
    add_files("kdbench.cpp")

mtb_plugin_target("tonemap")
    add_files("tonemap.cpp")
