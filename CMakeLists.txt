#/******************************************************************************
# Octaspire Dern - Programming language
# Copyright 2017 www.octaspire.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#******************************************************************************/
cmake_minimum_required(VERSION 2.8.12)
project(octaspire-dern C)

set(OCTASPIRE_DERN_CONFIG_VERSION_MAJOR 0)
set(OCTASPIRE_DERN_CONFIG_VERSION_MINOR 334)
set(OCTASPIRE_DERN_CONFIG_VERSION_PATCH 0)

message("== Building ${PROJECT_NAME} ${OCTASPIRE_DERN_CONFIG_VERSION_MAJOR}.${OCTASPIRE_DERN_CONFIG_VERSION_MINOR}.${OCTASPIRE_DERN_CONFIG_VERSION_PATCH}")

message("== Building ${PROJECT_NAME} in ${CMAKE_SYSTEM_NAME} system")

set(OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/test/resource/")


if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "Haiku" OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD" OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(OCTASPIRE_DERN_BINARY_PLUGINS   "OCTASPIRE_DERN_BINARY_PLUGINS"   ON)
else()
    option(OCTASPIRE_DERN_BINARY_PLUGINS   "OCTASPIRE_DERN_BINARY_PLUGINS"   OFF)
endif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "Haiku" OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD" OR CMAKE_SYSTEM_NAME STREQUAL "Linux")

if(OCTASPIRE_DERN_BINARY_PLUGINS)
    message("== Binary plugin system enabled for ${PROJECT_NAME}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-export-dynamic")
    set(OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS 1)
endif(OCTASPIRE_DERN_BINARY_PLUGINS)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/octaspire/dern/octaspire_dern_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/octaspire/dern/octaspire_dern_config.h")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel. " FORCE)
endif(NOT CMAKE_BUILD_TYPE)

message("== Build type of ${PROJECT_NAME} is ${CMAKE_BUILD_TYPE}")

if(EXISTS "${CMAKE_SOURCE_DIR}/.gitmodules")
    message("== Updating git submodules of ${PROJECT_NAME}...")
    message("-----------------------------------------------")
    execute_process(
        COMMAND git submodule update --init --recursive --remote
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    message("== Submodules of ${PROJECT_NAME} updated")
    message("-----------------------------------------------")
endif()

if(NOT TARGET octaspire-core)
    message("== Project ${PROJECT_NAME} adding subdirectory:")
    message("-----------------------------------------------")
    add_subdirectory(external/octaspire_core)
    message("-----------------------------------------------")
endif(NOT TARGET octaspire-core)

file(GLOB SOURCES               "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB HEADERS               "${CMAKE_CURRENT_SOURCE_DIR}/include/octaspire/dern/*.h")
file(GLOB BINARY_PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/etc/plugins/*.c")
file(GLOB MANUAL1               "${CMAKE_CURRENT_SOURCE_DIR}/doc/man/man1/octaspire-dern-repl.1")
file(GLOB MANUAL3               "${CMAKE_CURRENT_SOURCE_DIR}/doc/man/man3/octaspire-dern.3")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/octaspire_core/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")
add_library(octaspire-dern STATIC  ${SOURCES})
#Line below seems to require CMake version 2.8.12
target_include_directories(octaspire-dern PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include")
set_property(TARGET octaspire-dern PROPERTY C_STANDARD 99)
set_property(TARGET octaspire-dern PROPERTY C_EXTENSIONS OFF)

if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c99 -fno-omit-frame-pointer -Wall -Wextra -Werror")
endif(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_definitions(-DOCTASPIRE_CLANG_PRAGMAS_ENABLED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    #set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fcolor-diagnostics -fsanitize=undefined")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fcolor-diagnostics")
endif(CMAKE_C_COMPILER_ID MATCHES "Clang")

option(OCTASPIRE_DERN_COVERAGE  "OCTASPIRE_DERN_COVERAGE"  OFF)
option(OCTASPIRE_DERN_UNIT_TEST "OCTASPIRE_DERN_UNIT_TEST" ON)

if(OCTASPIRE_DERN_COVERAGE)
    message("== Code coverage enabled for ${PROJECT_NAME}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --coverage")
endif(OCTASPIRE_DERN_COVERAGE)

if(OCTASPIRE_DERN_UNIT_TEST)
    message("== Unit testing enabled for ${PROJECT_NAME}")
else()
    message("== Unit testing disabled for ${PROJECT_NAME}")
endif(OCTASPIRE_DERN_UNIT_TEST)

target_link_libraries(octaspire-dern octaspire-core m)

if(OCTASPIRE_DERN_BINARY_PLUGINS)
    if(UNIX)
        message("== Building binary plugins for ${PROJECT_NAME} in UNIX mode")
        if(NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Haiku" AND NOT CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
            message("== Building binary plugins for ${PROJECT_NAME} in NON FreeBSD, NON Haiku and NON NetBSD mode")
            target_link_libraries(octaspire-dern dl)
        endif(NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Haiku" AND NOT CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    endif(UNIX)
endif(OCTASPIRE_DERN_BINARY_PLUGINS)

add_executable(octaspire-dern-repl "src/octaspire_dern_repl.c")
target_link_libraries(octaspire-dern-repl octaspire-dern)
set_property(TARGET octaspire-dern-repl PROPERTY C_STANDARD 99)
install(TARGETS octaspire-dern DESTINATION lib)
install(TARGETS octaspire-dern-repl DESTINATION bin)
install(FILES ${HEADERS} DESTINATION "include/octaspire/dern")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/octaspire/dern/octaspire_dern_config.h" DESTINATION "include/octaspire/dern")
install(FILES ${MANUAL1} DESTINATION "man/man1")
install(FILES ${MANUAL3} DESTINATION "man/man3")
add_custom_target(
    cppcheck-dern
    COMMAND cppcheck --std=c99 -I "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/external/octaspire_core/include"
            --enable=warning,performance,portability
            --verbose --quiet ${SOURCES} ${HEADERS})

add_custom_target(
    vera-dern
    COMMAND vera++ --root "${CMAKE_CURRENT_SOURCE_DIR}/external/vera" --profile octaspire --error ${SOURCES} ${HEADERS})

add_custom_target(
    vera-dern-plugin
    COMMAND vera++ --root "${CMAKE_CURRENT_SOURCE_DIR}/external/vera" --profile octaspire-plugin --error ${BINARY_PLUGIN_SOURCES})

add_custom_target(
    new-release-dern
    COMMAND "${CMAKE_SOURCE_DIR}/etc/new-release.sh" "${CMAKE_SOURCE_DIR}")

add_custom_target(
    publish-release-dern
    COMMAND "${CMAKE_SOURCE_DIR}/etc/publish-release.sh" "${CMAKE_SOURCE_DIR}")

add_custom_target(
    codecov-dern
    COMMAND "${CMAKE_SOURCE_DIR}/etc/codecov.sh" "${CMAKE_SOURCE_DIR}")

add_custom_target(
    book-dern
    #COMMAND asciidoctor -a source-highlighter=pygments -a max-width=70em --backend=html5 "${CMAKE_CURRENT_SOURCE_DIR}/doc/book/Programming_in_Octaspire_Dern.adoc")
    COMMAND asciidoctor -r "${CMAKE_SOURCE_DIR}/etc/pygments_init.rb" -a source-highlighter=pygments --backend=html5 "${CMAKE_CURRENT_SOURCE_DIR}/doc/book/Programming_in_Octaspire_Dern.adoc"
    COMMAND asciidoctor-pdf -r "${CMAKE_SOURCE_DIR}/etc/pygments_init.rb" -a source-highlighter=pygments "${CMAKE_CURRENT_SOURCE_DIR}/doc/book/Programming_in_Octaspire_Dern.adoc")
    #COMMAND asciidoc -a source-highlighter=pygments --theme=flask --backend=html5 "${CMAKE_CURRENT_SOURCE_DIR}/doc/book/Programming_in_Octaspire_Dern.adoc")

if(NOT TARGET octaspire-dern-test-runner AND OCTASPIRE_DERN_UNIT_TEST)
    message("== Building of unit tests enabled for ${PROJECT_NAME}")
    enable_testing()
    include(CTest)
    add_subdirectory(test)
endif(NOT TARGET octaspire-dern-test-runner AND OCTASPIRE_DERN_UNIT_TEST)

