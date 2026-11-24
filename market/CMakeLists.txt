add_library(market MarketState.cpp)

target_include_directories(market PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(market PUBLIC util message)