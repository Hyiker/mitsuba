set_kind("shared")
add_deps("mitsuba-render")

target("addimages")
    add_files("addimages.cpp")
target("joinrgb")
    add_files("joinrgb.cpp")
target("cylclip")
    add_deps("mitsuba-hw")
    add_files("cylclip.cpp")
target("kdbench")
    add_files("kdbench.cpp")
target("tonemap")
    add_files("tonemap.cpp")
