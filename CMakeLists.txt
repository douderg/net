cmake_minimum_required (VERSION 3.7.0)
project (https-client)

find_package (Boost REQUIRED COMPONENTS system thread)
find_package (OpenSSL REQUIRED COMPONENTS SSL)
aux_source_directory (src SRC)
add_library (${PROJECT_NAME} ${SRC})
target_link_libraries (${PROJECT_NAME} PUBLIC Boost::system Boost::thread OpenSSL::SSL)
target_include_directories (${PROJECT_NAME} PUBLIC inc)
