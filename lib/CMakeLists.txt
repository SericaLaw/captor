file(GLOB_RECURSE LIB_SOURCES "*.cpp" "*.h" "buffer/*.cpp" "buffer/*.h" "reactor/*.cpp" "reactor/*.h")
include_directories(buffer reactor)
add_library(network_101 SHARED ${LIB_SOURCES})