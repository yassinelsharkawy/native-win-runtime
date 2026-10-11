include_guard(GLOBAL)

if(NOT CMAKE_HOST_SYSTEM_NAME MATCHES "Linux|Windows")
    message(FATAL_ERROR
        "This project must be configured from Linux or Windows with LLVM. "
        "Current host is '${CMAKE_HOST_SYSTEM_NAME}'."
    )
endif()

set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/override-link-rules.cmake")

set(CMAKE_SYSTEM_NAME Windows)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    find_program(CLANG_C_COMPILER NAMES clang clang-20 REQUIRED)
    find_program(CLANG_CXX_COMPILER NAMES clang++ clang++-20 REQUIRED)
else()
    find_program(CLANG_C_COMPILER NAMES clang-20 clang REQUIRED)
    find_program(CLANG_CXX_COMPILER NAMES clang++-20 clang++ REQUIRED)
endif()

set(CMAKE_C_COMPILER "${CLANG_C_COMPILER}")
set(CMAKE_CXX_COMPILER "${CLANG_CXX_COMPILER}")
set(CMAKE_ASM_COMPILER "${CLANG_C_COMPILER}")

set(CMAKE_MSVC_RUNTIME_LIBRARY "")

set(CMAKE_C_COMPILER_TARGET x86_64-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET x86_64-pc-windows-msvc)
set(CMAKE_ASM_COMPILER_TARGET x86_64-pc-windows-msvc)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(WIN_SDK_ROOT "C:/Program Files (x86)/Windows Kits/10" CACHE PATH "Path to Windows Kits 10")
    set(WIN_SDK_VER "10.0.26100.0" CACHE STRING "Windows SDK Version")
    set(MSVC_ROOT "C:/Program Files/Microsoft Visual Studio/18/Enterprise/VC/Tools/MSVC/14.51.36231" CACHE PATH "Path to MSVC toolchain")

    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
        "${MSVC_ROOT}/include"
        "${WIN_SDK_ROOT}/Include/${WIN_SDK_VER}/ucrt"
        "${WIN_SDK_ROOT}/Include/${WIN_SDK_VER}/shared"
        "${WIN_SDK_ROOT}/Include/${WIN_SDK_VER}/um"
    )
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_STANDARD_INCLUDE_DIRECTORIES})

    link_directories(
        "${MSVC_ROOT}/lib/x64"
        "${WIN_SDK_ROOT}/Lib/${WIN_SDK_VER}/um/x64"
        "${WIN_SDK_ROOT}/Lib/${WIN_SDK_VER}/ucrt/x64"
    )
else()
    set(XWIN_ROOT "$ENV{HOME}/.local/share/xwin" CACHE PATH "Path to the xwin installation root")

    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
        "${XWIN_ROOT}/crt/include"
        "${XWIN_ROOT}/sdk/include/ucrt"
        "${XWIN_ROOT}/sdk/include/shared"
        "${XWIN_ROOT}/sdk/include/um"
    )
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_STANDARD_INCLUDE_DIRECTORIES})

    link_directories(
        "${XWIN_ROOT}/crt/lib/x86_64"
        "${XWIN_ROOT}/sdk/lib/um/x86_64"
    )
endif()

set(NWR_PHNT_VERSION "PHNT_WINDOWS_10" CACHE STRING "PHNT target Windows version")

set(NWR_WIN32_WINNT "0x0A00" CACHE STRING "_WIN32_WINNT target version")

add_compile_definitions(
    PHNT_MODE=PHNT_MODE_USER
    PHNT_VERSION=${NWR_PHNT_VERSION}
    _WIN32_WINNT=${NWR_WIN32_WINNT}
    _AMD64_=1
)

add_compile_options(
    -march=skylake
    -ffunction-sections
    -fdata-sections
    -fvisibility=hidden
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
    -fno-ident
    -Wall
    -Wextra
    -Wpedantic
    -fno-stack-protector
    -fno-stack-check
    -mno-stack-arg-probe
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

add_link_options(
    -nostdlib
    -nodefaultlibs
    -nostartfiles
    -fno-rtlib-defaultlib
)
