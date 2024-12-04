link_libraries(nvbench::main)

find_package(hipblas REQUIRED)

add_executable(shared src/shared.hip)
add_executable(mat_mul src/mat_mul.hip)

target_link_libraries(mat_mul PRIVATE roc::hipblas)
