add_library(message FixMessageBuilder.cpp FixTagValueReader.cpp FixMessageParser.cpp Message.cpp)

target_include_directories(message PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(message PUBLIC util network)