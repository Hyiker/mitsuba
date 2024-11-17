add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")
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
mtb_plugin_target("ao")
    add_files("direct/ao.cpp")

mtb_plugin_target("direct")
    add_files("direct/direct.cpp")

mtb_plugin_target("path")
    add_files("path/path.cpp")

mtb_plugin_target("volpath")
    add_files("path/volpath.cpp")

mtb_plugin_target("volpath_simple")
    add_files("path/volpath_simple.cpp")

mtb_plugin_target("ptracer")
    add_files("ptracer/ptracer.cpp", "ptracer/ptracer_proc.cpp")

mtb_plugin_target("photonmapper")
    add_files("photonmapper/photonmapper.cpp", "photonmapper/bre.cpp")

mtb_plugin_target("ppm")
    add_files("photonmapper/ppm.cpp")

mtb_plugin_target("sppm")
    add_files("photonmapper/sppm.cpp")

mtb_plugin_target("vpl")
    add_files("vpl/vpl.cpp")

mtb_plugin_target("adaptive")
    add_files("misc/adaptive.cpp")

mtb_plugin_target("irrcache")  
    add_files("misc/irrcache.cpp", "misc/irrcache_proc.cpp")

mtb_plugin_target("multichannel")
    add_files("misc/multichannel.cpp")

mtb_plugin_target("field")
    add_files("misc/field.cpp")

mtb_plugin_target("motion")
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


mtb_plugin_target("bdpt")
    add_deps("mitsuba-bidir")
    add_files("bdpt/bdpt.cpp", "bdpt/bdpt_wr.cpp", "bdpt/bdpt_proc.cpp")

mtb_plugin_target("pssmlt")
    add_deps("mitsuba-bidir")
    add_files("pssmlt/pssmlt.cpp", "pssmlt/pssmlt_sampler.cpp", "pssmlt/pssmlt_proc.cpp")

mtb_plugin_target("mlt")
    add_deps("mitsuba-bidir")
    add_files("mlt/mlt.cpp", "mlt/mlt_proc.cpp")

mtb_plugin_target("erpt")
    add_deps("mitsuba-bidir")
    add_files("erpt/erpt.cpp", "erpt/erpt_proc.cpp")
