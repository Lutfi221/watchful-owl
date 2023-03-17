cmake_minimum_required(VERSION 3.11)
project(watchful-owl VERSION 1.1.0)

set(UI_SOURCE_FILES
  main/ui/browser.hpp
  main/ui/ui.cpp
  main/ui/pages.cpp
)

set(COMMON_SOURCE_FILES 
  main/helpers.cpp
  main/capturer.cpp
  main/config.cpp
  main/logger.cpp
  main/crypto.cpp
  main/constants.hpp
  main/autorun.cpp
  main/json.hpp
)

set(PERPETUAL_TARGET_NAME "perpetual-owl")

# --- Dependencies ---
include(cmake/CPM.cmake)
CPMAddPackage(
  "gh:ArthurSonzogni/FTXUI@3.0.0"
)
CPMAddPackage(
  "gh:gabime/spdlog@1.11.0"
)
CPMAddPackage(
  "gh:abdes/cryptopp-cmake#CRYPTOPP_8_7_0_1"
)
CPMAddPackage(
  "gh:andrew-hardin/cmake-git-version-tracking#7d9442bfade5a47b77291e4117f81cc65887c7b9"
)
# ---------------------

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(PERPETUAL_TARGET_NAME "${PERPETUAL_TARGET_NAME}-DEBUG")
endif()

add_compile_definitions(
  "PROJECT_VERSION=\"${CMAKE_PROJECT_VERSION}\""
  "DEBUG_BUILD=$<CONFIG:Debug>"
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
  PRIVATE spdlog
  PRIVATE cryptopp
  PRIVATE cmake_git_version_tracking
)

target_include_directories(watchful-owl PRIVATE main)

add_executable(
  ${PERPETUAL_TARGET_NAME} 
  WIN32
  main/perpetual.cpp
  ${COMMON_SOURCE_FILES}
)

target_link_libraries(
  ${PERPETUAL_TARGET_NAME}
  PRIVATE -lpsapi
  PRIVATE spdlog
  PRIVATE cryptopp
)

target_include_directories(${PERPETUAL_TARGET_NAME} PRIVATE main)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
