cmake_minimum_required(VERSION 3.16.3)


project(glfw3Learning)


set(pubinclude /usr/local)
set(CMAKE_INSTALL_PREFIX ${pubinclude})
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)



add_executable(glfw3Learning ./src/main.cpp)

set_property(TARGET glfw3Learning PROPERTY CXX_STANDARD 17)


target_include_directories(glfw3Learning PRIVATE ${pubinclude}/include)
target_include_directories(glfw3Learning PRIVATE ${pubinclude}/include/glfw)

target_link_libraries(glfw3Learning /System/Library/Frameworks/OpenGL.framework)
target_link_libraries(glfw3Learning /System/Library/Frameworks/Cocoa.framework)
target_link_libraries(glfw3Learning /System/Library/Frameworks/IOKit.framework)
target_link_libraries(glfw3Learning /System/Library/Frameworks/CoreVideo.framework)
target_link_libraries(glfw3Learning ${pubinclude}/lib/libglfw3.a)
target_link_libraries(glfw3Learning /usr/local/lib/libassimp.dylib)
