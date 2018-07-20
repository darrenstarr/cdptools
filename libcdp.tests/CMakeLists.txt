cmake_minimum_required(VERSION 2.8.2)

set(CMAKE_BUILD_TYPE Debug)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
#if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
#endif()

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(
    libcdptests
    test_cdp_packet.cpp
    test_software_version_string.cpp
    pch.cpp
    ../libcdp/buffer_stream.h
    ../libcdp/cdp_neighbor.h
    ../libcdp/cdp_packet.h
    ../libcdp/cdp_packet_parser.h
    ../libcdp/cdp_software_version_string.h
    ../libcdp/cisco_cluster_management_protocol.h
    ../libcdp/ecdpnetworkduplex.h
    ../libcdp/ecdptlv.h
    ../libcdp/ip_address_array.h
    ../libcdp/ip_prefix.h
    ../libcdp/ip_prefix_array.h
    ../libcdp/platform/checksum.h
    ../libcdp/platform/platform.h
    ../libcdp/platform/socket.h
    ../libcdp/platform/string.h
    ../libcdp/platform/time.h
    ../libcdp/platform/types.h
    ../libcdp/platform/utsname.h
    ../libcdp/power_over_ethernet_availability.h
    ../libcdp/stream_reader.h
    ../libcdp/stream_writer.h
    ../libcdp/buffer_stream.c
    ../libcdp/cdp_neighbor.c
    ../libcdp/cdp_packet.c
    ../libcdp/cdp_packet_parser.c
    ../libcdp/cdp_software_version_string_linux.c
    ../libcdp/cdp_software_version_string_windows.c
    ../libcdp/cisco_cluster_management_protocol.c
    ../libcdp/ip_address_array.c
    ../libcdp/ip_prefix.c
    ../libcdp/ip_prefix_array.c
    ../libcdp/power_over_ethernet_availability.c
    ../libcdp/stream_reader.c
    ../libcdp/stream_writer.c
)
target_link_libraries(libcdptests gtest_main)
