cmake_minimum_required (VERSION 3.7.0)
project (net)

if (MSVC)
    add_compile_options(-D_WIN32_WINNT=0x0601)
    add_compile_options(-bigobj)
endif ()

enable_testing()

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