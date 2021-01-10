cmake_minimum_required(VERSION 3.0.0)
project(RedBack-Media-Backend VERSION 0.1.0)

include(CTest)

enable_testing()


list(APPEND CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ~/.local/lib/cmake)
#set(CMAKE_CXX_COMPILER /usr/bin/clang++)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -lc++abi")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")

find_library(WEBRTC_LIB 
            NAME webrtc
            PATHS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/WebRTC-build/files/build/src/out/${CMAKE_BUILD_TYPE}/obj/
            NO_DEFAULT_PATH
            REQUIRED)

set(WEBRTC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/WebRTC-build/files/build/src)

#find_package(EventSocketCpp REQUIRED)
find_package(jsoncpp REQUIRED)

add_subdirectory(include)
add_subdirectory(thirdparty)

set(EXECUTABLE_OUTPUT_PATH ./${CMAKE_BUILD_TYPE})

#set(VIDEO_TRACK_SOURCE ./webrtc-checkout/src/pc/video_track_source.cc)
#set(WEBRTC_HEADERS webrtc-checkout/src)
set(ABSEIL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/abseil/include)

# find_library(WEBRTC_LIBRARY webrtc
#     PATHS
#         "${CMAKE_FIND_ROOT_PATH}/../webrtc-checkout/src/out/${CMAKE_BUILD_TYPE}/obj"
# )

file(
	GLOB_RECURSE
    MY_SOURCE_FILES
    src/server.h
    src/PeerConnectionCommon.h
    src/PeerConnectionHelpers.h
    src/PeerConnectionHelpers.cpp
    src/PeerConnectionObserverImp.h
    src/PeerConnectionObserverImp.cpp
    src/CreateSessionDescriptionObserverImp.cpp
    src/SetSessionDescriptionObserverImp.h
	src/VideoTrackSourceImp.h
    src/VideoCapturer.h
    src/client.h
)

add_library(${PROJECT_NAME}_LIB ${MY_SOURCE_FILES})
add_executable(${PROJECT_NAME}_SERVER src/mainServer.cpp)
add_executable(${PROJECT_NAME}_CLIENT src/mainClient.cpp)

target_include_directories(${PROJECT_NAME}_LIB
    PUBLIC 
    $<BUILD_INTERFACE:${WEBRTC_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${ABSEIL_HEADERS}>
    $<BUILD_INTERFACE:${jsoncpp_INCLUDE_DIRS}>
)

target_link_libraries(${PROJECT_NAME}_LIB -ljsoncpp -lpthread EventSocketCpp::eventsocketcpp ${WEBRTC_LIB} ${CMAKE_DL_LIBS})

target_compile_definitions(${PROJECT_NAME}_LIB PUBLIC WEBRTC_POSIX REDBACK_DEBUG CONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/config/config.txt")

target_link_libraries(${PROJECT_NAME}_SERVER ${PROJECT_NAME}_LIB)
target_link_libraries(${PROJECT_NAME}_CLIENT ${PROJECT_NAME}_LIB)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


