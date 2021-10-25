file(GLOB_RECURSE LIB_SOURCES "*.cpp" "*.h" "reactor/*.cpp" "reactor/*.h")

add_library(network_101 SHARED ${LIB_SOURCES})