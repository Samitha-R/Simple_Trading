add_library(log LogMessage.cpp BinaryLogger.cpp)

target_include_directories(log PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(log PUBLIC util)