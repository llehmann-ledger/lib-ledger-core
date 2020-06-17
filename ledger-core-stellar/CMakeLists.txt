cmake_minimum_required(VERSION 3.12)
project(ledger-core-stellar)

add_subdirectory(src)

if(BUILD_TESTS)
    message(STATUS "ledger-core-stellar tests are enabled")
    enable_testing()
    add_subdirectory(test)
else()
    message(STATUS "ledger-core-stellar tests are disabled")
endif()
