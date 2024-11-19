add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")

-- plugins += env.SharedLibrary('independent', ['independent.cpp'])
-- plugins += env.SharedLibrary('stratified', ['stratified.cpp'])
-- plugins += env.SharedLibrary('halton', ['halton.cpp', 'faure.cpp'])
-- plugins += env.SharedLibrary('hammersley', ['hammersley.cpp', 'faure.cpp'])
-- plugins += env.SharedLibrary('ldsampler', ['ldsampler.cpp'])
-- plugins += env.SharedLibrary('sobol', ['sobol.cpp', 'sobolseq.cpp'])
for _, fi in ipairs({"independent.cpp", "stratified.cpp", "ldsampler.cpp"}) do
    mtb_plugin_target(path.basename(fi))
        add_files(fi)
end

mtb_plugin_target("halton")
    add_files("halton.cpp", "faure.cpp")

mtb_plugin_target("hammersley")
    add_files("hammersley.cpp", "faure.cpp")

mtb_plugin_target("sobol")
    add_files("sobol.cpp", "sobolseq.cpp")