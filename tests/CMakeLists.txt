add_executable(test_ripeness test_ripeness.cpp)
target_link_libraries(test_ripeness PRIVATE vision_core)
add_test(NAME ripeness COMMAND test_ripeness)

add_executable(test_fruit_detector test_fruit_detector.cpp)
target_link_libraries(test_fruit_detector PRIVATE vision_core)
add_test(NAME fruit_detector COMMAND test_fruit_detector)

if(WIN32 AND VISION_RUNTIME_DIR AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.22")
    foreach(_t ripeness fruit_detector)
        set_tests_properties(${_t} PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${VISION_RUNTIME_DIR}")
    endforeach()
endif()
