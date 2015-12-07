cmake_minimum_required(VERSION 2.6)

project(sampleplayer)

# build static library
add_library(sampleplayer-static STATIC
  sampleplayer.c)

# build dynamic library
add_library(sampleplayer SHARED
  sampleplayer.c)
target_link_libraries(sampleplayer sndfile)

# build PD external
add_library(sampleplayer-pdext SHARED
  sampleplayer.c
  sampleplayer_pdext.c)
target_link_libraries(sampleplayer-pdext sndfile)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
