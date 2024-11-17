add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")

-- plugins += env.SharedLibrary('independent', ['independent.cpp'])
-- plugins += env.SharedLibrary('stratified', ['stratified.cpp'])
-- plugins += env.SharedLibrary('halton', ['halton.cpp', 'faure.cpp'])
-- plugins += env.SharedLibrary('hammersley', ['hammersley.cpp', 'faure.cpp'])
-- plugins += env.SharedLibrary('ldsampler', ['ldsampler.cpp'])
-- plugins += env.SharedLibrary('sobol', ['sobol.cpp', 'sobolseq.cpp'])
mtb_plugin_target("dipole")
    add_files("dipole.cpp", "irrproc.cpp", "irrtree.cpp", "bluenoise.cpp")

mtb_plugin_target("singlescatter")
    add_files("singlescatter.cpp")