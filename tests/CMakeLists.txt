
add_library(test_assets INTERFACE)
target_include_directories(test_assets INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(test_http http.cpp)
target_link_libraries(test_http net)
add_test(NAME test_http COMMAND $<TARGET_FILE:test_http>)

add_executable(test_https https.cpp)
target_link_libraries(test_https net test_assets)
add_test(NAME test_https COMMAND $<TARGET_FILE:test_https>)