cmake_minimum_required(VERSION 3.0.0)
project(RedBack-Media-Backend VERSION 0.1.0)

include(CTest)
enable_testing()

set(EXECUTABLE_OUTPUT_PATH ./${CMAKE_BUILD_TYPE})
set(BOOST_HEADERS boost_1_73_0)
set(WEBRTC_HEADERS webrtc-checkout/src)
set(ABSEIL_HEADERS abseil/include)
file(
    GLOB_RECURSE
    MY_SOURCE_FILES
    src/*
)

add_executable(${PROJECT_NAME} ${MY_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${BOOST_HEADERS} ${WEBRTC_HEADERS} ${ABSEIL_HEADERS})

target_link_libraries(${PROJECT_NAME} -ljsoncpp -lpthread)

target_compile_definitions(${PROJECT_NAME} PUBLIC WEBRTC_POSIX REDBACK_DEBUG)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
