add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")

-- testEnv = env.Clone()
-- testEnv.Append(CPPDEFINES = [['MTS_TESTCASE', '1']])

-- bidirEnv = testEnv.Clone()
-- bidirEnv.Append(LIBS=['mitsuba-bidir'])
-- bidirEnv.Append(LIBPATH=['#src/libbidir'])

-- for plugin in glob.glob(GetBuildPath('test_*.cpp')):
--         name = os.path.basename(plugin)
--         if "bidir" in name:
--                 lib = bidirEnv.SharedLibrary(name[0:len(name)-4], name)
--         else:
--                 lib = testEnv.SharedLibrary(name[0:len(name)-4], name)
--         if isinstance(lib, SCons.Node.NodeList):
--                 lib = lib[0]
--         plugins += [ lib ]


for _, fi in ipairs(os.files("test_*.cpp")) do
    mtb_plugin_target(path.basename(fi))
        add_defines("MTS_TESTCASE=1")
        add_files(fi)
        if fi:find("bidir") then
            add_deps("mitsuba-bidir")
        end
end
