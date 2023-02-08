cmake_minimum_required(VERSION 3.11)
project(watchful-owl VERSION 0.1.0)

set(UI_SOURCE_FILES
  main/ui/ui.cpp
)

set(COMMON_SOURCE_FILES 
  main/helpers.cpp
  main/capturer.cpp
  main/config.cpp
  main/logger.cpp
  main/constants.hpp
  main/json.hpp
)

set(PERPETUAL_TARGET_NAME "perpetual-owl")

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

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(PERPETUAL_TARGET_NAME "${PERPETUAL_TARGET_NAME}-DEBUG")
endif()

add_compile_definitions(
  "DEBUG=$<CONFIG:Debug>"
  "PERPETUAL_TARGET_NAME=\"${PERPETUAL_TARGET_NAME}\""
)

find_library(PSAPI Psapi)

add_executable(
  watchful-owl 
  main/main.cpp
  ${UI_SOURCE_FILES}
  ${COMMON_SOURCE_FILES}
)

target_link_libraries(watchful-owl
  PRIVATE -lpsapi
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

target_include_directories(watchful-owl PRIVATE main)

add_executable(
  ${PERPETUAL_TARGET_NAME} 
  main/perpetual.cpp
  ${COMMON_SOURCE_FILES}
)

target_link_libraries(
  ${PERPETUAL_TARGET_NAME}
  PRIVATE -lpsapi
)

target_include_directories(${PERPETUAL_TARGET_NAME} PRIVATE main)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
