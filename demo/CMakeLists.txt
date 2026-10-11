add_executable(nwr-demo
    main.cc
    demo_output.cc
)

target_link_libraries(nwr-demo PRIVATE
    nwr-syscall
    nwr-runtime
)

target_link_options(nwr-demo PRIVATE
    "LINKER:/entry:start"
    "LINKER:/map:$<TARGET_FILE_BASE_NAME:nwr-demo>.map"
    "LINKER:/opt:ref"
    "LINKER:/opt:icf"
    "LINKER:/timestamp:0"
    "LINKER:/lldsavetemps"
)

add_gnu_final_link(nwr-demo console start .exe EXE
    DEPENDENCIES nwr-syscall
    LIBRARIES ntdll
)
