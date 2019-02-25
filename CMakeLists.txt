set(PROJNAME "sfml_reliable_udp")
set(SER "server-example")
set(CLI "client-example")

cmake_minimum_required(VERSION 2.6)

project(${PROJNAME})

if (CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

set(CMAKE_DEBUG_POSTFIX "_d")

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/../bin")


set(HDRS
	./src/UdpServerBase.hpp
	./src/Connection.hpp
	./src/UdpClientBase.hpp
)

set(SRCS
	./src/main.cpp
	./src/UdpServerBase.cpp
	./src/Connection.cpp
	./src/UdpClientBase.cpp
)

#add_library(${PROJNAME} SHARED ${HDRS} ${SRCS})
add_executable(${PROJNAME} ${HDRS} ${SRCS})
target_link_libraries(${PROJNAME}
	-lpthread
	-lsfml-system
	-lsfml-network)
set_target_properties(${PROJNAME} PROPERTIES DEBUG_POSTFIX _d)


#set_target_properties(${PROJNAME} PROPERTIES PUBLIC_HEADER include/mylib.hpp)
#install(TARGETS ${PROJNAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set_target_properties(${PROJNAME} PROPERTIES DEBUG_POSTFIX _d)

target_link_libraries(${PROJNAME}) 

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)


if(MINGW OR UNIX)
        set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
endif(MINGW OR UNIX)
