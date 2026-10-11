#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "printf::printf" for configuration "Release"
set_property(TARGET printf::printf APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(printf::printf PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/printf.lib"
  )

list(APPEND _cmake_import_check_targets printf::printf )
list(APPEND _cmake_import_check_files_for_printf::printf "${_IMPORT_PREFIX}/lib/printf.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
