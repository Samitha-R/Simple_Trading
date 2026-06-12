add_library(execution ExecutionEngine.cpp BrokerProfile.cpp OrderManager.cpp SymbolState.cpp)

target_include_directories(execution PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(execution PUBLIC util)