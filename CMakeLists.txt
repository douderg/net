cmake_minimum_required (VERSION 3.7.0)
project (net)

find_package (Boost REQUIRED COMPONENTS system thread)
find_package (OpenSSL REQUIRED COMPONENTS SSL)

aux_source_directory (src SRC)
add_library (${PROJECT_NAME} ${SRC})
if (WIN32)
    target_link_libraries (${PROJECT_NAME} PUBLIC Boost::system Boost::thread OpenSSL::SSL Crypt32)
else ()
    target_link_libraries (${PROJECT_NAME} PUBLIC Boost::system Boost::thread OpenSSL::SSL)
endif ()
target_include_directories (${PROJECT_NAME} PUBLIC inc)

add_subdirectory(tests)