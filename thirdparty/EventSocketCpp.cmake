# EventSocketCpp fetch content
cmake_minimum_required(VERSION 3.15)
include(FetchContent)

FetchContent_Declare(
    eventsocketcpp 
    GIT_REPOSITORY  https://github.com/Yousif-CS/eventsocketcpp.git
    GIT_TAG         main
)

FetchContent_MakeAvailable(eventsocketcpp)
