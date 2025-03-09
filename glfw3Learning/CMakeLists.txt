cmake_minimum_required(VERSION 3.16.3)

project(glfw3Learning)
set(CMAKE_STATIC_LINKER_FLAGS "-liconv")

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

add_executable(glfw3Learning ./src/main.cpp)

set_property(TARGET glfw3Learning PROPERTY CXX_STANDARD 17)

target_include_directories(glfw3Learning PRIVATE ${pubinclude}/include)
target_include_directories(glfw3Learning PRIVATE ${Homebrew}/libiconv/include)
target_include_directories(glfw3Learning PRIVATE ./include)
target_include_directories(glfw3Learning PRIVATE ${pubApp}/Autodesk/FBX\ SDK/2020.3.7/include)
target_include_directories(glfw3Learning PRIVATE ${Homebrew}/libxml2/include)
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
