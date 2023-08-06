add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")

-- plugins += env.SharedLibrary('area', ['area.cpp'])
-- plugins += env.SharedLibrary('collimated', ['collimated.cpp'])
-- plugins += env.SharedLibrary('directional', ['directional.cpp'])
-- plugins += env.SharedLibrary('point', ['point.cpp'])
-- plugins += env.SharedLibrary('constant', ['constant.cpp'])
-- plugins += env.SharedLibrary('envmap', ['envmap.cpp'])
-- plugins += env.SharedLibrary('sky', ['sky.cpp', 'sunsky/skymodel.cpp'])
-- plugins += env.SharedLibrary('sun', ['sun.cpp'])
-- plugins += env.SharedLibrary('sunsky', ['sunsky.cpp'])
-- plugins += env.SharedLibrary('spot', ['spot.cpp'])
for _, fi in ipairs({"area.cpp", "collimated.cpp", "directional.cpp", "point.cpp",
 "constant.cpp", "envmap.cpp", "sun.cpp", "sunsky.cpp", "spot.cpp"}) do
    target(path.basename(fi))
        set_kind("shared")
        add_files(fi)
end

target("sky")
    set_kind("shared")
    add_files("sky.cpp", "sunsky/skymodel.cpp")