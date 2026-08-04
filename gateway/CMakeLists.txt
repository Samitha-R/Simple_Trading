add_library(gateway SessionWrapper.cpp)

target_include_directories(gateway PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gateway PUBLIC util network message market)