add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include> 
)
