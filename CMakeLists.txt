# CMakeList.txt : CMake project for afc_sync, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("afc_sync")

# Add source to this project's executable.
add_executable (afc_sync "afc_sync.c" )

# TODO: Add tests and install targets if needed.
if (WIN32)
find_library(DLLS NAMES imobiledevice PATHS lib)
message(STATUS "find_library returned ${DLLS}")
add_library(imobiledevice SHARED IMPORTED)
set_target_properties(imobiledevice PROPERTIES IMPORTED_LOCATION "${DLLS}" IMPORTED_IMPLIB "${DLLS}")
set_target_properties(imobiledevice PROPERTIES LINKER_LANGUAGE C)
include_directories(include)

target_link_libraries(afc_sync imobiledevice)
endif (WIN32)

if (UNIX)
include(FindPkgConfig)
find_package(PkgConfig REQUIRED)
pkg_search_module(IMD REQUIRED IMPORTED_TARGET libimobiledevice-1.0)
include_directories(${IMD_INCLUDE_DIRS})
target_link_libraries(afc_sync ${IMD_LDFLAGS})

endif (UNIX)

