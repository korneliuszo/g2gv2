cmake_minimum_required(VERSION 3.3)
project (g2gv2)
add_executable(g2gv2
	main.cpp
	GerbvDecoder.cpp
	Contour.cpp
	PngRender.cpp
	Fill.cpp
	GcodePrinter.cpp
	Optimize.cpp
)
set_property(TARGET g2gv2 PROPERTY CXX_STANDARD 17)
target_include_directories (g2gv2 PRIVATE "include")

find_package(PkgConfig REQUIRED)
pkg_search_module(libgerbv REQUIRED libgerbv)
target_compile_options(g2gv2 PUBLIC ${libgerbv_CFLAGS_OTHER})
target_include_directories(g2gv2 SYSTEM PUBLIC ${libgerbv_INCLUDE_DIRS})
target_link_libraries(g2gv2 LINK_PUBLIC ${libgerbv_LIBRARIES})

find_package(PNG REQUIRED)
target_compile_options(g2gv2 PUBLIC ${PNG_DEFINITIONS})
target_include_directories(g2gv2 SYSTEM PUBLIC ${PNG_INCLUDE_DIRS})
target_link_libraries(g2gv2 LINK_PUBLIC ${PNG_LIBRARY})

FIND_PACKAGE( Boost 1.70 COMPONENTS program_options REQUIRED )
target_include_directories(g2gv2 SYSTEM PUBLIC ${Boost_INCLUDE_DIR})
TARGET_LINK_LIBRARIES( g2gv2 LINK_PUBLIC ${Boost_LIBRARIES} )

set(CMAKE_BUILD_TYPE "RelWithDebInfo")
#set(CMAKE_BUILD_TYPE Debug)
