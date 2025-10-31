cmake_minimum_required(VERSION 3.30)
project(voxel_mania)

set(CMAKE_CXX_STANDARD 23)

if((CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo") AND MSVC)
    if(EXISTS "C:/rad_linker/radlink.exe")
        set(CMAKE_LINKER "C:/rad_linker/radlink.exe")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
        add_link_options(/INCREMENTAL:NO)
        message(STATUS "Using RAD Linker for ${CMAKE_BUILD_TYPE} builds")
    else()
        message(WARNING "RAD Linker not found at C:/rad_linker/radlink.exe, using default MSVC linker")
    endif()
endif()




add_subdirectory(libraries)
add_subdirectory(primitive_mesh_data)
find_package(OpenGL REQUIRED)

add_executable(voxel_mania
        main.cpp
        Camera.cpp
        InputHandling.cpp
        InputHandling.h
        AppData.h
        math_ops.h
        math_ops.cpp
        RandomNumberGeneration.cpp
        RandomNumberGeneration.h
        Perlin.h
        Perlin.cpp
        Renderer/Renderer.cpp
)

target_link_libraries(voxel_mania PUBLIC primitive_mesh_data)

target_link_libraries(voxel_mania PRIVATE
        libglew_shared
        SDL3::SDL3
        LinearAlgebra
        Trigonometry
        DearImGui
        OpenGL::GL
        OpenGlHelpers
        stb_image
)

# Copy shaders
add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:voxel_mania>/data"
        COMMENT "Copying shaders"
)
add_dependencies(voxel_mania copy_shaders)

# Automatically copy all runtime shared libraries (cross-platform)
if(WIN32)
add_custom_command(TARGET voxel_mania POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:voxel_mania>
        $<TARGET_FILE_DIR:voxel_mania>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime shared libraries"
)
endif()

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

