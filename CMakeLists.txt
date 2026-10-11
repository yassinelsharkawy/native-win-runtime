cmake_minimum_required(VERSION 3.20)

project(native-win-runtime
    VERSION 1.0.0
    LANGUAGES CXX ASM
    DESCRIPTION "Modular Windows x64 Native API & CRT-Free Runtime Framework"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/gnu-link.cmake)

option(NWR_BUILD_DEMO "Build the demonstration executable" ON)
option(NWR_BUILD_TESTS "Build tests" ON)
option(NWR_BUILD_RUNTIME "Build the CRT-free runtime" ON)

# --- Syscall dispatch library ---

add_library(nwr-syscall STATIC
    src/nt_syscall_bridge.S
    src/status.cc
    src/syscall_resolver.cc
)

target_include_directories(nwr-syscall
    PUBLIC
        include
)

# --- Third-party dependencies ---

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/phnt/CMakeLists.txt")
    message(FATAL_ERROR
        "phnt submodule is not initialised.\n"
        "Run:  git submodule update --init --recursive"
    )
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/printf/CMakeLists.txt")
    message(FATAL_ERROR
        "printf submodule is not initialised.\n"
        "Run:  git submodule update --init --recursive"
    )
endif()

add_subdirectory(third_party/phnt)

get_target_property(PHNT_INCLUDE_DIRS phnt INTERFACE_INCLUDE_DIRECTORIES)
set_property(TARGET phnt APPEND PROPERTY
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    "${PHNT_INCLUDE_DIRS}"
)

target_link_libraries(nwr-syscall
    PUBLIC
        phnt::phnt
)

# --- CRT-free runtime ---

if(NWR_BUILD_RUNTIME)
    add_subdirectory(runtime)
endif()

# --- Demo ---

if(NWR_BUILD_DEMO)
    add_subdirectory(demo)
endif()

# --- Tests ---

if(NWR_BUILD_TESTS)
    add_subdirectory(tests)
endif()
