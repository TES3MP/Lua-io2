include(LibFindMacros)

libfind_pkg_detect(LuaJit luajit
    FIND_PATH luajit.h
        HINTS $ENV{LUA_DIR}
        PATH_SUFFIXES include include/luajit-2.1
    FIND_LIBRARY luajit-5.1 luajit
        HINTS $ENV{LUA_DIR}
        PATH_SUFFIXES lib
)


libfind_process(LuaJit)

message(STATUS "LuaJit ASDASD ${LuaJit_LIBRARIES}")

set(LUA_LIBRARY ${LuaJit_LIBRARY})
set(LUA_INCLUDE_DIR ${LuaJit_INCLUDE_DIR})
