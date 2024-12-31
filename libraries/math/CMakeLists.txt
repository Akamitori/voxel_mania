add_library(LinearAlgebra SHARED Vector2D.cpp)
add_library(Trigonometry SHARED Trigonometry.cpp)


install(TARGETS LinearAlgebra Trigonometry
        RUNTIME DESTINATION ${BINARY_DIRECTORY}  # DLLs
        LIBRARY DESTINATION ${BINARY_DIRECTORY_LIBS}  # .so/.dylib
)