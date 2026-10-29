cmake_minimum_required(VERSION 3.16)

project(BaitapChuoiXanhChin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(OpenCV REQUIRED)

set(VISION_RUNTIME_DIR "")
if(WIN32 AND DEFINED OpenCV_DIR)
    get_filename_component(_vision_ocv_prefix "${OpenCV_DIR}/../../.." ABSOLUTE)
    if(EXISTS "${_vision_ocv_prefix}/bin")
        set(VISION_RUNTIME_DIR "${_vision_ocv_prefix}/bin")
    endif()
endif()

add_library(vision_core
    src/ripeness.cpp
    src/fruit_detector.cpp
)
target_include_directories(vision_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(vision_core PUBLIC ${OpenCV_LIBS})
target_compile_features(vision_core PUBLIC cxx_std_17)

add_executable(detect src/main.cpp)
target_link_libraries(detect PRIVATE vision_core)

option(BUILD_TESTING "Build unit tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
