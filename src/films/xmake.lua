add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")
for _, fi in ipairs("ldrfilm.cpp", "hdrfilm.cpp") do
    target(path.basename(fi))
        set_kind("shared")
        add_files(fi)
end

target("mfilm")
    set_kind("shared")
    add_files("mfilm.cpp", "cnpy.cpp")

target("tiledhdrfilm")
    set_kind("shared")
    add_files("tiledhdrfilm.cpp")