set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_COLOR_DIAGNOSTICS ON)
# set(CMAKE_EXPORT_BUILD_DATABASE ON) # Causes Configuration Error?
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Conflict with CMAKE_UNITY_BUILD
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set(CMAKE_UNITY_BUILD ON)
# set(CMAKE_UNITY_BUILD_BATCH_SIZE 8)

qgc_enable_pie()
qgc_enable_ipo()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    if(NOT APPLE)
        qgc_set_linker()
    endif()
    add_link_options("$<$<CONFIG:Release>:-flto=thin>")
elseif(MSVC)
    add_link_options("$<$<CONFIG:Release>:/LTCG:INCREMENTAL>")
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:Embedded>")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(LINUX)
        # set(ENV{DESTDIR} "${CMAKE_BINARY_DIR}/staging")
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/AppDir/usr" CACHE PATH "Install path prefix for AppImage" FORCE)
    else()
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/staging" CACHE PATH "Install path prefix" FORCE)
    endif()
endif()

if(CMAKE_CROSSCOMPILING)
    if(NOT IS_DIRECTORY ${QT_HOST_PATH})
        message(FATAL_ERROR "You need to set QT_HOST_PATH to cross compile Qt.")
    endif()

    if(ANDROID)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    endif()
endif()

if(APPLE AND NOT IOS)
    set(MACOS TRUE)

    # if(CMAKE_APPLE_SILICON_PROCESSOR MATCHES "arm64")
    # if(CMAKE_APPLE_SILICON_PROCESSOR MATCHES "x86_64")
    # if("${CMAKE_OSX_ARCHITECTURES}" MATCHES "arm64;x86_64" OR "${CMAKE_OSX_ARCHITECTURES}" MATCHES "x86_64;arm64")
    #     set(QGC_MACOS_UNIVERSAL_BUILD ON)
    # endif()
endif()
