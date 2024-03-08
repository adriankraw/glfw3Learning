cmake_minimum_required(VERSION 3.16.3)

project(glfw3Learning)

set(pubinclude /usr/local)
set(CMAKE_INSTALL_PREFIX ${oubinclude})
add_executable(glfw3Learning ./src/main.cpp)
target_include_directories(glfw3Learning PRIVATE ${pubinclude}/include)
target_link_libraries(glfw3Learning /usr/local/lib/libassimp.dylib)
