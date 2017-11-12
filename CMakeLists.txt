cmake_minimum_required(VERSION 3.8)
project(io2)

if (CMAKE_CXX_COMPILE_FEATURES MATCHES cxx_std_17)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 14)
    #set(Boost_USE_STATIC_LIBS   ON)
    find_package(Boost REQUIRED)
    find_package(Boost COMPONENTS system filesystem REQUIRED)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(LuaJit)


set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

include_directories("." SYSTEM
        "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/sol2"
        ${LuaJit_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        )

set(SOURCE_FILES library.cpp library.h)
add_library(io2 SHARED ${SOURCE_FILES})

include(GenerateExportHeader)
generate_export_header(io2)

set_target_properties(io2 PROPERTIES PREFIX "")

target_link_libraries(io2 ${LuaJit_LIBRARY} ${Boost_LIBRARIES})

if (UNIX AND CMAKE_CXX_COMPILE_FEATURES MATCHES cxx_std_17)
    target_link_libraries(io2 stdc++fs)
endif()

add_custom_target(BasicTest
        DEPENDS io2
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tests"
        COMMAND ${CMAKE_COMMAND} -E env LUA_CPATH="${CMAKE_BINARY_DIR}/?.so\;${CMAKE_BINARY_DIR}/?.dll"
        lua "${CMAKE_SOURCE_DIR}/tests/basic.lua"
        )

add_custom_target(Example
        DEPENDS io2
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tests"
        COMMAND ${CMAKE_COMMAND} -E env LUA_CPATH="${CMAKE_BINARY_DIR}/?.so\;${CMAKE_BINARY_DIR}/?.dll"
        lua "${CMAKE_SOURCE_DIR}/tests/example.lua"
        )