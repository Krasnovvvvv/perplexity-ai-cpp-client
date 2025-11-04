cmake_minimum_required(VERSION 3.15)
project(perplexity-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PERPLEXITY_BUILD_EXAMPLES "Build example programs" ON)
option(PERPLEXITY_BUILD_TESTS "Build tests" OFF)

find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(perplexity INTERFACE)
add_library(perplexity::perplexity ALIAS perplexity)

target_include_directories(perplexity INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(perplexity INTERFACE
        CURL::libcurl
        nlohmann_json::nlohmann_json
)

target_compile_features(perplexity INTERFACE cxx_std_17)


if(PERPLEXITY_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(PERPLEXITY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()


install(DIRECTORY include/perplexity
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)

install(TARGETS perplexity
        EXPORT perplexity-targets
        INCLUDES DESTINATION include
)

install(EXPORT perplexity-targets
        FILE perplexity-targets.cmake
        NAMESPACE perplexity::
        DESTINATION share/perplexity/cmake
)


include(CMakePackageConfigHelpers)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/perplexity-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/perplexity-config.cmake"
        INSTALL_DESTINATION share/perplexity/cmake
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/perplexity-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/perplexity-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/perplexity-config-version.cmake"
        DESTINATION share/perplexity/cmake
)