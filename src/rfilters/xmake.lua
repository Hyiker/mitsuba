add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")
for _, fi in ipairs(os.files("*.cpp")) do
    target(path.basename(fi))
        set_kind("shared")
        add_files(fi)
end