add_library(strategy StrategyEngine.cpp)

target_include_directories(strategy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} #${PROJECT_SOURCE_DIR}/gateway
)

target_link_libraries(strategy PUBLIC util market gateway)