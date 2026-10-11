cmake_minimum_required(VERSION 3.20)

block()
    set(BUILD_SHARED_LIBS OFF)
    set(ALIAS_STANDARD_FUNCTION_NAMES HARD)

    add_subdirectory(
        "${CMAKE_SOURCE_DIR}/third_party/printf"
        "${CMAKE_CURRENT_BINARY_DIR}/printf"
    )
endblock()

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    src/*.cc
)

add_library(nwr-runtime OBJECT
    ${SRC_FILES}
)

target_include_directories(nwr-runtime
    PUBLIC
        include
)

target_link_libraries(nwr-runtime
    PUBLIC
        phnt::phnt
        printf
)

target_compile_options(printf PRIVATE
    -Wno-everything
)

target_compile_definitions(nwr-runtime PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)
