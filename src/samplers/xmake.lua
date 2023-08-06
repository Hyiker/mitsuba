add_deps("mitsuba-render", "mitsuba-core", "mitsuba-hw")

-- plugins += env.SharedLibrary('independent', ['independent.cpp'])
-- plugins += env.SharedLibrary('stratified', ['stratified.cpp'])
-- plugins += env.SharedLibrary('halton', ['halton.cpp', 'faure.cpp'])
-- plugins += env.SharedLibrary('hammersley', ['hammersley.cpp', 'faure.cpp'])
-- plugins += env.SharedLibrary('ldsampler', ['ldsampler.cpp'])
-- plugins += env.SharedLibrary('sobol', ['sobol.cpp', 'sobolseq.cpp'])
for _, fi in ipairs({"independent.cpp", "stratified.cpp", "ldsampler.cpp"}) do
    target(path.basename(fi))
        set_kind("shared")
        add_files(fi)
end

target("halton")
    set_kind("shared")
    add_files("halton.cpp", "faure.cpp")

target("hammersley")
    set_kind("shared")
    add_files("hammersley.cpp", "faure.cpp")

target("sobol")
    set_kind("shared")
    add_files("sobol.cpp", "sobolseq.cpp")