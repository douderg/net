
add_library(test_assets INTERFACE)
target_include_directories(test_assets INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(test_http http.cpp)
target_link_libraries(test_http net)
add_test(NAME test_http COMMAND $<TARGET_FILE:test_http>)

add_executable(test_https https.cpp)
target_link_libraries(test_https net test_assets)
add_test(NAME test_https COMMAND $<TARGET_FILE:test_https>)

add_executable(test_ws ws.cpp)
target_link_libraries(test_ws net)
add_test(NAME test_ws COMMAND $<TARGET_FILE:test_ws>)

add_executable(test_wss wss.cpp)
target_link_libraries(test_wss net test_assets)
add_test(NAME test_wss COMMAND $<TARGET_FILE:test_wss>)