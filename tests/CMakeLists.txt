set(TEST_SOURCE_FILES)
include_directories(../include)
include_directories(./googletest/include)

add_subdirectory(./tokenizer)

find_package(Threads REQUIRED)

add_subdirectory(./googletest)
set(TEST_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/list-tests.cc ${TEST_SOURCE_FILES})
add_executable(jast_tests ${TEST_SOURCE_FILES})
target_link_libraries(jast_tests jast Threads::Threads gtest gtest_main)
