cmake_minimum_required(VERSION 3.30)
project(voxel_mania)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(libraries)
find_package(OpenGL REQUIRED)

add_executable(voxel_mania
        main.cpp
        Camera.cpp
        Camera.h
        data/cube.h
        data/triangle.h
        InputHandling.cpp
        InputHandling.h
        AppData.h
        math_ops.h
        math_ops.cpp
        RandomNumberGeneration.cpp
        RandomNumberGeneration.h
        Perlin.h
        Perlin.cpp
)

target_link_libraries(voxel_mania PRIVATE
        libglew_shared
        SDL3::SDL3
        LinearAlgebra
        Trigonometry
        DearImGui
        OpenGL::GL
)

# Copy shaders
add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:voxel_mania>/data"
        COMMENT "Copying shaders"
)
add_dependencies(voxel_mania copy_shaders)

## Copy shared libraries to exe directory (cross-platform)
#add_custom_command(TARGET voxel_mania POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE:SDL3::SDL3>
#        $<TARGET_FILE:libglew_shared>
#        $<TARGET_FILE:LinearAlgebra>
#        $<TARGET_FILE:Trigonometry>
#        $<TARGET_FILE_DIR:voxel_mania>
#        COMMENT "Copying shared libraries to executable directory"
#)

# Automatically copy all runtime shared libraries (cross-platform)
add_custom_command(TARGET voxel_mania POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:voxel_mania>
        $<TARGET_FILE_DIR:voxel_mania>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime shared libraries"
)

# Install executable
install(TARGETS voxel_mania RUNTIME DESTINATION . COMPONENT Application)

# Install ALL runtime dependencies automatically (same as POST_BUILD)
install(FILES $<TARGET_RUNTIME_DLLS:voxel_mania> DESTINATION . COMPONENT Application)

# Install data folder  
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/" DESTINATION data COMPONENT Application)

# Clean up GLEW's mess
install(CODE "
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/lib\")
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/include\")
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/cmake\")
")

