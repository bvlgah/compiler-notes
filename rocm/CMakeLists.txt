add_executable(shared src/shared.hip)
target_link_libraries(shared nvbench::main)
