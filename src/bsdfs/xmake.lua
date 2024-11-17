add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")
for _, fi in ipairs(os.files("*.cpp")) do
    mtb_plugin_target(path.basename(fi))
        add_files(fi)
end