add_executable(week05 main.cpp indexed_mesh.cpp shader_program.cpp)
target_link_libraries(week05 PRIVATE cg_deps)
target_compile_definitions(week05 PRIVATE
    WEEK05_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
