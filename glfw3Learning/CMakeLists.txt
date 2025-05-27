cmake_minimum_required(VERSION 3.16.3)

project(glfw3Learning)
set(CMAKE_STATIC_LINKER_FLAGS "-liconv")
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(BUILD_MODE Debug)
SET(CMAKE_CXX_FLAGS "-Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(ICONV_DIR /opt/homebrew/opt/libiconv)
set(Iconv_LIBRARY /opt/homebrew/opt/libiconv/lib/libiconv.a)
set(Iconv::Iconv /opt/homebrew/opt/libiconv/lib/libiconv.a)
set(Iconv_INCLUDE_DIR /opt/homebrew/opt/libiconv/include)

find_package(Iconv MODULE REQUIRED)

set(pubinclude /usr/local)
set(frameworks /System/Library/Frameworks)
set(pubApp /Applications)
set(Homebrew /opt/homebrew/opt)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(IMGUI 
        extern/imgui/imgui_impl_opengl3.h
        extern/imgui/imgui_impl_opengl3.cpp
        extern/imgui/imgui_impl_opengl3_loader.h
        extern/imgui/imgui_impl_glfw.h
        extern/imgui/imgui_impl_glfw.cpp
        extern/imgui/imgui.h
        extern/imgui/imgui.cpp
        extern/imgui/imgui_draw.cpp
        extern/imgui/imgui_tables.cpp
        extern/imgui/imgui_widgets.cpp
        extern/imgui/imgui_demo.cpp
)
set(STB include/stb/stb_image.h)

add_library(libz STATIC IMPORTED)
set_target_properties(libz PROPERTIES
	IMPORTED_LOCATION "${Homebrew}/zlib/lib/libz.a"
	INTERFACE_INCLUDE_DIRECTORIES "${Homebrew}/zlib/include"
)

add_library(fbxsdk STATIC IMPORTED)
set_target_properties(fbxsdk PROPERTIES
	IMPORTED_LOCATION "${pubApp}/Autodesk/FBX SDK/2020.3.7/lib/clang/release/libfbxsdk.a"
	INTERFACE_INCLUDE_DIRECTORIES "${pubApp}/Autodesk/FBX SDK/2020.3.7/include"
)

add_executable(glfw3Learning ./src/main.cpp ${IMGUI} ${STB})

set_property(TARGET glfw3Learning PROPERTY CXX_STANDARD 17)

target_include_directories(glfw3Learning PRIVATE ${pubinclude}/include)
target_include_directories(glfw3Learning PRIVATE ${Homebrew}/libiconv/include)
target_include_directories(glfw3Learning PRIVATE ./include)
target_include_directories(glfw3Learning PRIVATE ${pubApp}/Autodesk/FBX\ SDK/2020.3.7/include)
target_include_directories(glfw3Learning PRIVATE ${Homebrew}/libxml2/include)
target_include_directories(glfw3Learning PRIVATE ./extern/imgui)
target_include_directories(glfw3Learning PRIVATE ./extern/stb)
#target_include_directories(glfw3Learning PRIVATE ${pubinclude}/include/glfw)


target_link_libraries(glfw3Learning Iconv)
target_link_libraries(glfw3Learning ${frameworks}/OpenGL.framework)
target_link_libraries(glfw3Learning ${frameworks}/Cocoa.framework)
target_link_libraries(glfw3Learning ${frameworks}/IOKit.framework)
target_link_libraries(glfw3Learning ${frameworks}/CoreVideo.framework)
target_link_libraries(glfw3Learning ${pubinclude}/lib/libglfw3.a)
target_link_libraries(glfw3Learning "${pubApp}/Autodesk/FBX\ SDK/2020.3.7/lib/clang/release/libalembic.a")
target_link_libraries(glfw3Learning "${Homebrew}/libxml2/lib/libxml2.dylib")
target_link_libraries(glfw3Learning libz)
target_link_libraries(glfw3Learning fbxsdk)
