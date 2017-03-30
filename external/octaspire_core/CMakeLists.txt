cmake_minimum_required(VERSION 3.5.1)

set(OCTASPIRE_CORE_CONFIG_VERSION_MAJOR 0)
set(OCTASPIRE_CORE_CONFIG_VERSION_MINOR 1)
set(OCTASPIRE_CORE_CONFIG_VERSION_PATCH 0)

set(OCTASPIRE_CORE_CONFIG_TEST_RES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/test/resource/")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/octaspire/core/octaspire_core_config.h.in"
    "${CMAKE_BINARY_DIR}/include/octaspire/core/octaspire_core_config.h")


project(octaspire_core C)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
        "Choode the type of build, options are: Debug Release RelWithDebInfo MinSizeRel. " FORCE)
endif(NOT CMAKE_BUILD_TYPE)
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB EXTERNAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/external/*.c")
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/octaspire/core/*.h")
file(GLOB MANUAL3 "${CMAKE_CURRENT_SOURCE_DIR}/doc/man/man3/octaspire-core.3")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories("${CMAKE_BINARY_DIR}/include")
add_library(octaspire_core STATIC  ${SOURCES} ${EXTERNAL_SOURCES})
set_property(TARGET octaspire_core PROPERTY C_STANDARD 99)
set_property(TARGET octaspire_core PROPERTY C_EXTENSIONS OFF)

if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -Wall -Wextra -pedantic -Werror")
endif(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fcolor-diagnostics -fsanitize=undefined,address")
endif(CMAKE_C_COMPILER_ID MATCHES "Clang")

if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -coverage -fprofile-arcs -ftest-coverage")
endif(CMAKE_C_COMPILER_ID MATCHES "GNU")

target_link_libraries(octaspire_core m)
install(TARGETS octaspire_core DESTINATION lib)
install(FILES ${HEADERS} DESTINATION "include/octaspire/core")
install(FILES "${CMAKE_BINARY_DIR}/include/octaspire/core/octaspire_core_config.h" DESTINATION "include/octaspire/core")
install(FILES ${MANUAL3} DESTINATION "man/man3")
add_custom_target(
    cppcheck-core
    COMMAND cppcheck --std=c99 -I "${CMAKE_CURRENT_SOURCE_DIR}/include"
            --enable=warning,performance,portability
            --verbose --quiet ${SOURCES} ${HEADERS})

add_custom_target(
    vera-core
    COMMAND vera++ --error ${SOURCES} ${HEADERS})

enable_testing()
include(CTest)
add_subdirectory(test)
