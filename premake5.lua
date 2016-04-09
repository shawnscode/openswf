workspace( "dependencies" )
    configurations( "release" )
    defines({ "NDEBUG" })
    optimize( "On" )
    location( "build/3rd" )

    project("tess2")
        kind( "StaticLib" )
        language( "C" )
        includedirs({ "3rd/libtess2/Include", "3rd/libtess2/Source" })
        files({ "3rd/libtess2/Source/*.c" })
        targetdir( "libs/3rd" )

workspace("openswf")
    configurations({ "debug", "release" })
    location( "build" )

    filter("configurations:debug")
        defines({ "DEBUG" })
        flags({ "Symbols" })

    filter("configurations:release")
        defines({ "NDEBUG" })
        optimize( "On" )

    project("openswf")
        kind( "StaticLib" )
        language( "C++" )
        buildoptions({"-std=c++11", "-stdlib=libc++"})
        libdirs({ "libs/3rd", "/usr/local/lib/" })
        -- links({ "tess2", "glfw3" })
        files({ "source/*.cpp" })
        includedirs({ "3rd/libtess2/Include", "/usr/local/include", "source" })
        targetdir( "libs" )

workspace("test")
    configurations( "debug" )
    location( "build/test" )
    defines({ "DEBUG" })
    flags({ "Symbols" })
    kind( "ConsoleApp" )
    libdirs({ "libs/3rd", "libs", "/usr/local/lib/" })
    includedirs({ "3rd/catch/include", "3rd/libtess2/Include", "/usr/local/include", "source", "test/00-common" })
    language( "C++" )
    buildoptions({"-std=c++11", "-stdlib=libc++"})
    targetdir( "bin" )

    project( "01-unit-test" )
        links({ "tess2", "glfw3", "glew", "openswf" })
        linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }
        files({ "test/01-unit-test/*.cpp", "test/00-common/*.cpp" })

    project("02-simple-shape")
        links({ "tess2", "glfw3", "glew", "openswf" })
        linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }
        files({ "test/02-simple-shape/*.cpp", "test/00-common/*.cpp", "source/*.cpp" })

    project("03-simple-timeline")
        links({ "tess2", "glfw3", "glew", "openswf" })
        linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }
        files({ "test/03-simple-timeline/*.cpp", "test/00-common/*.cpp" })
