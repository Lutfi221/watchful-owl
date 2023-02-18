cmake_minimum_required(VERSION 3.11)
project(watchful-owl VERSION 0.1.0)

set(UI_SOURCE_FILES
  main/ui/browser.cpp
  main/ui/ui.cpp
)

set(COMMON_SOURCE_FILES 
  main/helpers.cpp
  main/capturer.cpp
  main/config.cpp
  main/logger.cpp
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
  NAME libtommath
  GITHUB_REPOSITORY libtom/libtommath
  VERSION 1.2.1
  # 1.2.1 is not released yet, so use a recent commit
  GIT_TAG 03de03dee753442d4b23166982514639c4ccbc39 
)
CPMAddPackage(
  NAME libtomcrypt
  GITHUB_REPOSITORY libtom/libtomcrypt
  VERSION 1.18.3
  # 1.18.3 is not released yet, so use a recent commit
  GIT_TAG 2a1b284677a51f587ab7cd9d97395e0c0c93a447 
)
# ---------------------

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(PERPETUAL_TARGET_NAME "${PERPETUAL_TARGET_NAME}-DEBUG")
endif()

add_compile_definitions(
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
  PRIVATE libtommath 
  PRIVATE libtomcrypt
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
  PRIVATE libtommath 
  PRIVATE libtomcrypt
)

target_include_directories(${PERPETUAL_TARGET_NAME} PRIVATE main)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
