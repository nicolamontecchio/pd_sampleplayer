cmake_minimum_required(VERSION 2.6)
project(sampleplayer)

cmake_policy(SET CMP0042 NEW)                      # silence warning (no idea what that is)

set(CMAKE_BUILD_TYPE "Release")

# flags needed for libsndfile and puredata include paths
set(CMAKE_C_FLAGS "-I/usr/local/include -I/Applications/Pd-0.49-0.app/Contents/Resources/src")
set(CMAKE_SHARED_LINKER_FLAGS "-L/usr/local/lib")

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
