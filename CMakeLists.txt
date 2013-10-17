cmake_minimum_required(VERSION 2.6)

PROJECT(plumage_twitter)

message(${CMAKE_SYSTEM})
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    include_directories(/usr/include /usr/local/include ./)
    link_directories(/usr/lib /usr/local/lib)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    include_directories(/opt/local/include /usr/include /usr/local/include ./)
    link_directories(/opt/local/lib /usr/lib /usr/local/lib)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

add_library(plumage_twitter SHARED
    src/plumage_twitter.cpp
    )

add_library(plumage_twitter_st STATIC
    src/plumage_twitter.cpp
    )

make_directory(lib)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set_target_properties(plumage_twitter plumage_twitter_st PROPERTIES COMPILE_FLAGS "-Wall -I ./include -std=c++11 -stdlib=libc++")
set_target_properties(plumage_twitter PROPERTIES LINK_FLAGS "-lplumage -lc++ -ldl")
set_target_properties(plumage_twitter_st PROPERTIES LINK_FLAGS "-lplumage -lc++ -ldl")
set_target_properties(plumage_twitter PROPERTIES LIBRARY_OUTPUT_DIRECTORY "lib/")
set_target_properties(plumage_twitter_st PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "lib/")

INSTALL(TARGETS plumage_twitter_st LIBRARY ARCHIVE DESTINATION lib)
INSTALL(TARGETS plumage_twitter LIBRARY DESTINATION lib)

