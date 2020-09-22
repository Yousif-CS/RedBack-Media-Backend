cmake_minimum_required(VERSION 3.0.0)
project(RedBack-Media-Backend VERSION 0.1.0)

include(CTest)
enable_testing()

set(EXECUTABLE_OUTPUT_PATH ./${CMAKE_BUILD_TYPE})
set(BOOST_HEADERS ./boost_1_73_0)

file(
    GLOB_RECURSE
    MY_SOURCE_FILES
    src/*
)

add_executable(RedBack-Media-Backend ${MY_SOURCE_FILES})

target_include_directories(RedBack-Media-Backend PRIVATE ${BOOST_HEADERS})

target_link_libraries(RedBack-Media-Backend -ljsoncpp -lpthread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
