# Three exercises share the scene; only the camera controls differ.
foreach(task RANGE 1 3)
    add_executable(week09_task${task} main.cpp)
    target_link_libraries(week09_task${task} PRIVATE cg_deps)
    target_compile_definitions(week09_task${task} PRIVATE WEEK09_TASK=${task})
endforeach()

if(BUILD_TESTING)
    add_executable(week09_camera_tests camera_tests.cpp)
    target_link_libraries(week09_camera_tests PRIVATE glm::glm)
    add_test(NAME week09_camera COMMAND week09_camera_tests)
endif()
