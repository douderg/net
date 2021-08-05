

add_executable(test_http http.cpp)
target_link_libraries(test_http net)
add_test(NAME test_http COMMAND $<TARGET_FILE:test_http>)

add_executable(test_https https.cpp)
target_link_libraries(test_https net)
add_test(NAME test_https COMMAND $<TARGET_FILE:test_https>)