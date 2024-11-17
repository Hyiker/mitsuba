set_kind("binary")
add_deps("mitsuba-core", "mitsuba-render")
if is_plat("macosx") then
    stubs = "darwin_stub.mm"
    add_ldflags("-Xlinker", "-rpath", "-Xlinker", "@executable_path/../Frameworks")
end

mtb_target("mtsutil")
    add_files("mtsutil.cpp", winstubs, stubs)

mtb_target("mitsuba")
    add_deps("data")
    add_files("mitsuba.cpp", winstubs)

mtb_target("mtssrv")
    add_files("mtssrv.cpp", winstubs)
