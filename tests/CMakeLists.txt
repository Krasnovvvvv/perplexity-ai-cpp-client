
enable_testing()
find_package(GTest CONFIG REQUIRED)

set(TESTS
        perplexity_tests.cpp
)

foreach(TEST_FILE ${TESTS})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest_main perplexity)
    target_compile_features(${TEST_NAME} PRIVATE cxx_std_17)
    target_include_directories(${TEST_NAME} PRIVATE
            ${PROJECT_SOURCE_DIR}/include
    )
    include(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
endforeach()
