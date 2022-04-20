cmake_minimum_required(VERSION 3.10)
project(ledger-app-hedera-unit-tests C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CMOCKA REQUIRED cmocka)

enable_testing()
include(CTest)

include_directories(../src)

add_library(hedera_format SHARED ../src/hedera_format.c)

add_executable(test_hedera_format test_hedera_format.c)
target_link_libraries(test_hedera_format ${CMOCKA_LIBRARIES} hedera_format)
target_include_directories(test_hedera_format PUBLIC ${CMOCKA_INCLUDE_DIRS})
target_compile_options(test_hedera_format PUBLIC ${CMOCKA_CFLAGS_OTHER})
target_link_directories(test_hedera_format PUBLIC ${CMOCKA_LIBRARY_DIRS})
add_test(test_hedera_format ${CMAKE_CURRENT_BINARY_DIR}/test_hedera_format)
