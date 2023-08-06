add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")
set_kind("shared")
-- plugins += env.SharedLibrary('ao', ['direct/ao.cpp'])
-- plugins += env.SharedLibrary('direct', ['direct/direct.cpp'])
-- plugins += env.SharedLibrary('path', ['path/path.cpp'])
-- plugins += env.SharedLibrary('volpath', ['path/volpath.cpp'])
-- plugins += env.SharedLibrary('volpath_simple', ['path/volpath_simple.cpp'])
-- plugins += env.SharedLibrary('ptracer', ['ptracer/ptracer.cpp', 'ptracer/ptracer_proc.cpp'])

-- # Photon mapping-based techniques
-- plugins += env.SharedLibrary('photonmapper', ['photonmapper/photonmapper.cpp', 'photonmapper/bre.cpp'])
-- plugins += env.SharedLibrary('ppm', ['photonmapper/ppm.cpp'])
-- plugins += env.SharedLibrary('sppm', ['photonmapper/sppm.cpp'])

-- # Miscellaneous
-- plugins += env.SharedLibrary('vpl', ['vpl/vpl.cpp'])
-- plugins += env.SharedLibrary('adaptive', ['misc/adaptive.cpp'])
-- plugins += env.SharedLibrary('irrcache', ['misc/irrcache.cpp', 'misc/irrcache_proc.cpp'])
-- plugins += env.SharedLibrary('multichannel', ['misc/multichannel.cpp'])
-- plugins += env.SharedLibrary('field', ['misc/field.cpp'])
-- plugins += env.SharedLibrary('motion', ['misc/motion.cpp'])
target("ao")
    add_files("direct/ao.cpp")

target("direct")
    add_files("direct/direct.cpp")

target("path")
    add_files("path/path.cpp")

target("volpath")
    add_files("path/volpath.cpp")

target("volpath_simple")
    add_files("path/volpath_simple.cpp")

target("ptracer")
    add_files("ptracer/ptracer.cpp", "ptracer/ptracer_proc.cpp")

target("photonmapper")
    add_files("photonmapper/photonmapper.cpp", "photonmapper/bre.cpp")

target("ppm")
    add_files("photonmapper/ppm.cpp")

target("sppm")
    add_files("photonmapper/sppm.cpp")

target("vpl")
    add_files("vpl/vpl.cpp")

target("adaptive")
    add_files("misc/adaptive.cpp")

target("irrcache")  
    add_files("misc/irrcache.cpp", "misc/irrcache_proc.cpp")

target("multichannel")
    add_files("misc/multichannel.cpp")

target("field")
    add_files("misc/field.cpp")

target("motion")
    add_files("misc/motion.cpp")

-- bidirEnv = env.Clone()
-- bidirEnv.Append(LIBS=['mitsuba-bidir'])
-- bidirEnv.Append(LIBPATH=['#src/libbidir'])

-- plugins += bidirEnv.SharedLibrary('bdpt',
--         ['bdpt/bdpt.cpp', 'bdpt/bdpt_wr.cpp', 'bdpt/bdpt_proc.cpp'])

-- plugins += bidirEnv.SharedLibrary('pssmlt',
--         ['pssmlt/pssmlt.cpp', 'pssmlt/pssmlt_sampler.cpp',
--     'pssmlt/pssmlt_proc.cpp']);

-- plugins += bidirEnv.SharedLibrary('mlt',
--         ['mlt/mlt.cpp', 'mlt/mlt_proc.cpp']
-- )

-- plugins += bidirEnv.SharedLibrary('erpt',
--         ['erpt/erpt.cpp', 'erpt/erpt_proc.cpp']
-- )


target("bdpt")
    add_deps("mitsuba-bidir")
    add_files("bdpt/bdpt.cpp", "bdpt/bdpt_wr.cpp", "bdpt/bdpt_proc.cpp")

target("pssmlt")
    add_deps("mitsuba-bidir")
    add_files("pssmlt/pssmlt.cpp", "pssmlt/pssmlt_sampler.cpp", "pssmlt/pssmlt_proc.cpp")

target("mlt")
    add_deps("mitsuba-bidir")
    add_files("mlt/mlt.cpp", "mlt/mlt_proc.cpp")

target("erpt")
    add_deps("mitsuba-bidir")
    add_files("erpt/erpt.cpp", "erpt/erpt_proc.cpp")
