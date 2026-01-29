include(../../cmake_scripts/link_and_export_utilities.cmake)

add_and_export_library(Custom_Containers TYPE INTERFACE)


add_and_export_library(Custom_Containers_Test TYPE SHARED FILES benchmarks/vector_int.cpp)
target_link_libraries(Custom_Containers_Test PRIVATE Custom_Containers)


add_executable(queue_tests benchmarks/queue_benchmarks.cpp)
target_link_libraries(queue_tests PRIVATE Custom_Containers)

# For a specific target
set_target_properties(queue_tests PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
)


# Static inline version (header-only)
add_executable(vector_tests_inline benchmarks/vector_benchmark.cpp)
target_link_libraries(vector_tests_inline PRIVATE Custom_Containers)

# Library version (linked)
add_executable(vector_tests_linked benchmarks/vector_benchmark.cpp)
target_link_libraries(vector_tests_linked PRIVATE Custom_Containers_Test)

target_compile_definitions(vector_tests_linked PRIVATE VECTOR_USE_LIBRARY)

set_target_properties(vector_tests_inline vector_tests_linked PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
)

