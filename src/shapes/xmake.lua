add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")
-- plugins += env.SharedLibrary('obj', ['obj.cpp'])

-- plugins += env.SharedLibrary('serialized', ['serialized.cpp'])
-- plugins += env.SharedLibrary('rectangle', ['rectangle.cpp'])
-- plugins += env.SharedLibrary('disk', ['disk.cpp'])
-- plugins += env.SharedLibrary('sphere', ['sphere.cpp'])
-- plugins += env.SharedLibrary('cylinder', ['cylinder.cpp'])
-- plugins += env.SharedLibrary('hair', ['hair.cpp'])
-- plugins += env.SharedLibrary('shapegroup', ['shapegroup.cpp'])
-- plugins += env.SharedLibrary('instance', ['instance.cpp'])
-- plugins += env.SharedLibrary('cube', ['cube.cpp'])
-- plugins += env.SharedLibrary('heightfield', ['heightfield.cpp'])
-- plugins += env.SharedLibrary('ply', ['ply.cpp', 'ply/ply_parser.cpp'],
--         CPPPATH = env['CPPPATH'] + ['#src/shapes'])

for _, fi in ipairs({"obj.cpp", "serialized.cpp", "rectangle.cpp", "disk.cpp",
 "sphere.cpp", "cylinder.cpp", "hair.cpp", "shapegroup.cpp", "instance.cpp",
  "cube.cpp", "heightfield.cpp"}) do
    mtb_plugin_target(path.basename(fi))
        add_files(fi)
end

mtb_plugin_target("ply")
    add_files("ply.cpp", "ply/ply_parser.cpp")
    add_includedirs(".")