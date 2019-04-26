include_directories(.)
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/link.T")

add_library(gba_senpai SHARED libretro/libretro.cpp)

set_target_properties(gba_senpai PROPERTIES PREFIX "")
set(CMAKE_EXE_LINKER_FLAGS "-T ${LINKER_SCRIPT}")

set_source_files_properties(libretro/libretro.cpp
PROPERTIES OBJECT_DEPENDS ${LINKER_SCRIPT})
