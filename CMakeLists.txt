cmake_minimum_required(VERSION 3.0.0)
project(watchful-owl VERSION 0.1.0)

include(CTest)
enable_testing()

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

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
