add_executable(nwr-tests
    test_status.cc
    test_runtime_helpers.cc
    test_resolver.cc
    ${CMAKE_SOURCE_DIR}/demo/demo_output.cc
)

target_link_libraries(nwr-tests PRIVATE
    nwr-syscall
    nwr-runtime
)

target_link_options(nwr-tests PRIVATE
    "LINKER:/entry:start"
    "LINKER:/opt:ref"
    "LINKER:/opt:icf"
    "LINKER:/timestamp:0"
    "LINKER:/lldsavetemps"
)

add_gnu_final_link(nwr-tests console start .exe EXE
    DEPENDENCIES nwr-syscall
    LIBRARIES ntdll
)
