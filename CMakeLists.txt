cmake_minimum_required(VERSION 3.16)
project(network_101)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib)
include_directories(lib lib/reactor lib/buffer)

add_executable(test_server.out test_server.cpp)
add_executable(test_client.out test_client.cpp)
target_link_libraries(test_server.out network_101)
target_link_libraries(test_client.out network_101)

add_executable(echo_poll_server.out echo_poll_server.cpp)
add_executable(echo_epoll_server.out echo_epoll_server.cpp)
target_link_libraries(echo_poll_server.out network_101)
target_link_libraries(echo_epoll_server.out network_101)

add_executable(simple_poll_reactor.out simple_poll_reactor.cpp)
add_executable(simple_epoll_reactor.out simple_epoll_reactor.cpp)
target_link_libraries(simple_poll_reactor.out network_101)
target_link_libraries(simple_epoll_reactor.out network_101)

add_executable(reactor_with_dispatcher.out reactor_with_dispatcher.cpp)
target_link_libraries(reactor_with_dispatcher.out network_101)

add_executable(reactor_multithread.out reactor_multithread.cpp)
target_link_libraries(reactor_multithread.out network_101 pthread)