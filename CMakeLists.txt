cmake_minimum_required(VERSION 3.11)
project(watchful-owl VERSION 0.1.0)

# --- Dependencies ---
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
  URL https://github.com/ArthurSonzogni/FTXUI/archive/refs/tags/v3.0.0.tar.gz
  URL_HASH MD5=b29f0f9eae43fdf1ce001131e9751893
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
# ---------------------

find_library(PSAPI Psapi)

add_executable(
  watchful-owl 
  main/main.cpp
  main/helpers.cpp
  main/capturer.cpp
  main/config.cpp
  main/logger.cpp
  main/json.hpp
  main/cppath.hpp
)

target_link_libraries(watchful-owl
  PRIVATE -lpsapi
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

target_include_directories(watchful-owl PRIVATE main)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
