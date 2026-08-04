cmake_minimum_required(VERSION 3.10)

# set the project name
project(Simple_Trading)

# If no build type is specified, default to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_GLIBCXX_DEBUG)
endif()

set(CMAKE_CXX_COMPILER g++)        # Use GCC

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



# subdirectoties
add_subdirectory(network)
add_subdirectory(util)
add_subdirectory(strategy)
add_subdirectory(message)
add_subdirectory(log)
add_subdirectory(market)
add_subdirectory(execution)
add_subdirectory(gateway)

# add the executable
add_executable(Simple_Trading main.cpp Dispatcher.cpp SymbolIds.cpp Updaters.cpp)
target_link_libraries(Simple_Trading PRIVATE network)
target_link_libraries(Simple_Trading PRIVATE util)
target_link_libraries(Simple_Trading PRIVATE strategy)
target_link_libraries(Simple_Trading PRIVATE message)
target_link_libraries(Simple_Trading PRIVATE log)
target_link_libraries(Simple_Trading PRIVATE market)
target_link_libraries(Simple_Trading PRIVATE execution)
target_link_libraries(Simple_Trading PRIVATE gateway)

