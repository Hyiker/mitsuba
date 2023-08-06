target("mitsuba-hw")
    set_kind("shared")
    add_files("*.cpp")

    add_defines("MTS_BUILD_MODULE=MTS_MODULE_HW")
    remove_files("wglsession.cpp", "wgldevice.cpp", "wglrenderer.cpp",
     "x11session.cpp", "x11device.cpp", "glxdevice.cpp", "glxrenderer.cpp")
    -- 'opengl32', 'glu32', 'glew32mx', 'gdi32', 'user32']
    add_syslinks("opengl32", "glu32", "gdi32", "user32")
    if is_plat("windows") then
        add_files("wglsession.cpp", "wgldevice.cpp", "wglrenderer.cpp")
    elseif is_plat("linux") then
        add_files("x11session.cpp", "x11device.cpp", "glxdevice.cpp", "glxrenderer.cpp")
    end
