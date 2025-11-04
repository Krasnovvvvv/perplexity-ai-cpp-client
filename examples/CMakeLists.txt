set(EXAMPLES
       example.cpp
)

foreach(EXAMPLE_FILE ${EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE perplexity)
    target_compile_features(${EXAMPLE_NAME} PRIVATE cxx_std_17)
    target_include_directories(${EXAMPLE_NAME} PRIVATE
            ${PROJECT_SOURCE_DIR}/include
    )
endforeach()
