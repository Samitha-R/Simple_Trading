add_library(util CommonUtils.cpp Message.cpp)

target_include_directories(util PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)