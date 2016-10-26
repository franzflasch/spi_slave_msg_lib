cmake_minimum_required(VERSION 2.8.9)
project(spi_slave_msg_lib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(LIB_SRCS spi_slave_messaging.c)

add_library(spi_slave_msg_lib SHARED ${LIB_SRCS})
include_directories(.)
set_target_properties(spi_slave_msg_lib PROPERTIES PUBLIC_HEADER spi_slave_messaging.h)
set_target_properties(spi_slave_msg_lib PROPERTIES SOVERSION "1")

install(TARGETS spi_slave_msg_lib DESTINATION lib PUBLIC_HEADER DESTINATION inc)

### Debian Package generation
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Franz Flasch") #required
include(CPack)
