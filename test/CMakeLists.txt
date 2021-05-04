
add_executable(test_simple_server test_simple_server.cpp)
add_executable(test_swcparser test_swcparser.cpp)
target_link_libraries(test_simple_server PRIVATE
        Poco::Net
        Poco::Util
        )