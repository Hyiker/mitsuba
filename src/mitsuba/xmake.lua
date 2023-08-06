set_kind("binary")
add_deps("mitsuba-core", "mitsuba-render")
set_languages("cxx17")
if is_plat("macosx") then
    stubs = "darwin_stub.mm"
    add_ldflags("-Xlinker", "-rpath", "-Xlinker", "@executable_path/../Frameworks")
end

target("mtsutil")
    add_files("mtsutil.cpp", winstubs, stubs)

target("mitsuba")
    add_deps("data")
    add_files("mitsuba.cpp", winstubs)

target("mtssrv")
    add_files("mtssrv.cpp", winstubs)
