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
project(octaspire-dern-test-runner C)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resource" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
file(GLOB SOURCES "*.c")
include_directories(${OCTASPIRE_DERN_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")
add_executable(octaspire-dern-test-runner ${SOURCES})
set_property(TARGET octaspire-dern-test-runner PROPERTY C_STANDARD 99)
set_property(TARGET octaspire-dern-test-runner PROPERTY C_EXTENSIONS OFF)

if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -pedantic")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c99 -Wall -Wextra -pedantic -Werror")
endif(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")

if(OCTASPIRE_DERN_COVERAGE)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --coverage")
endif(OCTASPIRE_DERN_COVERAGE)

if(NOT WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GREATEST_ENABLE_ANSI_COLORS=1)
    message("== Enabled ANSI colors for ${PROJECT_NAME}")
endif(NOT WIN32)

target_link_libraries(octaspire-dern-test-runner octaspire-dern)

add_test(octaspire-dern-test-runner octaspire-dern-test-runner)
