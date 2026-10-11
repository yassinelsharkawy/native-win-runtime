function(add_gnu_final_link
    target
    subsystem
    entry
    output_suffix
    pe_kind
)
    cmake_parse_arguments(
        PARSE_ARGV 5
        ARG
        ""
        ""
        "DEPENDENCIES;LIBRARIES"
    )

    if(pe_kind STREQUAL "DLL")
        set(_pe_type_options --dll)
    elseif(pe_kind STREQUAL "EXE")
        set(_pe_type_options)
    else()
        message(FATAL_ERROR
            "add_gnu_final_link: pe_kind must be EXE or DLL, got: ${pe_kind}"
        )
    endif()

    set(_gnu_linker_script
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gnu-final.ld"
    )

    set(_gnu_output
        "${CMAKE_CURRENT_BINARY_DIR}/${target}.ld${output_suffix}"
    )

    set(_map_file
        "${CMAKE_CURRENT_BINARY_DIR}/${target}.ld.map"
    )

    set(_dependency_files)
    foreach(_dependency IN LISTS ARG_DEPENDENCIES)
        get_target_property(_dependency_type "${_dependency}" TYPE)
        list(APPEND _dependency_files "$<TARGET_FILE:${_dependency}>")
    endforeach()

    set(_lib_args)
    foreach(_lib IN LISTS ARG_LIBRARIES)
        list(APPEND _lib_args "-l${_lib}")
    endforeach()

    find_program(GNU_LD_EXECUTABLE
        NAMES x86_64-w64-mingw32-ld ld.bfd ld
        PATHS C:/mingw64/bin /usr/bin
    )
    if(NOT GNU_LD_EXECUTABLE)
        message(FATAL_ERROR "add_gnu_final_link: Could not find x86_64-w64-mingw32-ld or ld")
    endif()

    if(WIN32)
        set(_mingw_lib_dir "C:/mingw64/x86_64-w64-mingw32/lib")
    else()
        set(_mingw_lib_dir "/usr/x86_64-w64-mingw32/lib")
    endif()

    add_custom_command(
        OUTPUT
        "${_gnu_output}"
        "${_map_file}"

        COMMAND
        "${GNU_LD_EXECUTABLE}"
        -T "${_gnu_linker_script}"
        ${_pe_type_options}
        --subsystem "${subsystem}"
        -e "${entry}"
        --gc-sections
        --no-insert-timestamp
        --section-alignment=0x1000
        --file-alignment=0x1000
        --strip-all
        --nostdlib
        "-Map=${_map_file}"
        "$<TARGET_FILE:${target}>.lto.obj"
        ${_dependency_files}
        -L "${_mingw_lib_dir}"
        ${_lib_args}
        -o "${_gnu_output}"

        DEPENDS
        ${target}
        ${ARG_DEPENDENCIES}
        "${_gnu_linker_script}"

        COMMENT
        "GNU final link ${target}"

        VERBATIM
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(
        ${target}-gnu-link
        ALL
        DEPENDS
        "${_gnu_output}"
        "${_map_file}"
    )
endfunction()
