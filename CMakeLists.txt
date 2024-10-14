cmake_minimum_required(VERSION 3.20)
project(nsissqliteplugin)

find_library(LIB_NSISAPI NAMES pluginapi-x86-unicode PATHS ${CMAKE_CURRENT_SOURCE_DIR}/nsis)

add_library(nsissqliteplugin SHARED
  nsissqliteplugin.c
  sqlite/sqlite3.c
)

target_include_directories(nsissqliteplugin PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/nsis
  ${CMAKE_CURRENT_SOURCE_DIR}/sqlite
)

target_compile_definitions(nsissqliteplugin PRIVATE UNICODE _UNICODE)

target_link_libraries(nsissqliteplugin ${LIB_NSISAPI})
target_link_options(nsissqliteplugin PRIVATE -static-libgcc -static-libstdc++)
set_target_properties(nsissqliteplugin PROPERTIES PREFIX "")
set_target_properties(nsissqliteplugin PROPERTIES OUTPUT_NAME "nsissqliteplugin")
