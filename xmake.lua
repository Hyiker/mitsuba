add_rules("mode.debug", "mode.release")

includes("src/libcore")
includes("src/librender")
includes("src/libhw")
includes("src/libbidir")
-- includes("src/libpython")

-- build the applications
-- Build the command-line binaries
includes("src/mitsuba")

includes("src")

includes("data")

package("xerces-c_custom")
    set_base("xerces-c")
    -- add_urls("https://github.com/apache/xerces-c.git")
    add_links("xerces-c_3")
package_end()


-- common configurations
function set_common()
    set_warnings("all")
    add_includedirs("include")
    add_languages("cxx17")

    add_defines("MTS_HAS_COHERENT_RT", "MTS_SSE", "SINGLE_PRECISION", "SPECTRUM_SAMPLES=3", 
    "MTS_HAS_LIBJPEG", "MTS_HAS_LIBPNG", "MTS_HAS_OPENEXR", "MTS_HAS_FFTW",
    "_HAS_AUTO_PTR_ETC=1")


    if is_mode("debug") then
        add_defines("MTS_DEBUG")
    else
        if is_plat("windows") then
            add_defines("NDEBUG")
        end
    end

    -- platform specified
    if is_plat("windows") then
        -- add_defines("UNICODE", "_UNICODE")
        add_cxxflags("/execution-charset:utf-8", "/source-charset:utf-8", "/openmp")
        add_defines("WIN32", "WIN64", "_CONSOLE", "OPENEXR_DLL")
        add_syslinks("msvcrt", "ws2_32")
        add_shflags("/nologo", "/SUBSYSTEM:CONSOLE", "/MACHINE:X64",
         "/FIXED:NO", "/OPT:REF", "/OPT:ICF", "/LTCG", "/NODEFAULTLIB:library", "/MANIFEST")
        winstubs = "$(projectdir)/data/windows/wmain_stub.cpp"
        resources = "$(projectdir)/data/windows/mitsuba_res.rc"
    end
end

-- dependencies
function set_dependencies()
    add_requires("zlib v1.2.13", "libjpeg-turbo 2.1.4", "libpng v1.6.40")
    add_requires("glew 2.2.0", "eigen 3.3.9", "fftw 3.3.10")

    add_requires("boost 1.81.0", {configs = {shared = true, thread = true, python = true, system = true, filesystem = true}})
    add_requires("freeglut v3.4.0", {configs = {shared = true}})
    add_requires("openexr 2.5.7", {configs = {build_both = true}})
    add_requires("xerces-c_custom 3.2.4", {configs = {shared = true}})

    add_packages("zlib", "openexr", "libjpeg-turbo", "libpng", "boost", "xerces-c_custom", "glew", "eigen", "fftw")
end

-- for file in os.listdir(env.GetBuildPath('#data/schema')):
--         if fnmatch.fnmatch(file, '*.xsl') or fnmatch.fnmatch(file, '*.xsd'):
--                 installTargets += env.Install(os.path.join(distDir, 'data/schema'), '#data/schema/' + file)
-- for file in os.listdir(env.GetBuildPath('#data/ior')):
--         if fnmatch.fnmatch(file, '*.spd'):
--                 installTargets += env.Install(os.path.join(distDir, 'data/ior'), '#data/ior/' + file)
-- for file in os.listdir(env.GetBuildPath('#data/microfacet')):
--         if fnmatch.fnmatch(file, '*.dat'):
--                 installTargets += env.Install(os.path.join(distDir, 'data/microfacet'), '#data/microfacet/' + file)

set_common()
set_dependencies()
