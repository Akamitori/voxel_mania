#!/bin/bash
# debug.sh - Build and launch RAD Debugger

set -e  # Exit on error

# Paths
PROJECT_DIR="C:/Users/PETROS/source/personal_projects/voxel_mania"
BUILD_DIR="$PROJECT_DIR/cmake-build-debug"
CMAKE_EXE="C:/Program Files/JetBrains/CLion 2025.2.1/bin/cmake/win/x64/bin/cmake.exe"
NINJA_EXE="C:/Program Files/JetBrains/CLion 2025.2.1/bin/ninja/win/x64/ninja.exe"
RADDBG_EXE="C:/rad_linker/raddbg.exe"  # Adjust to your path
TARGET_NAME="voxel_mania"

echo "=== Configuring CMake ==="
"$CMAKE_EXE" \
    -DCMAKE_BUILD_TYPE=Debug \
    "-DCMAKE_MAKE_PROGRAM=$NINJA_EXE" \
    -G Ninja \
    -S "$PROJECT_DIR" \
    -B "$BUILD_DIR"

echo ""
echo "=== Building ==="
"$CMAKE_EXE" --build "$BUILD_DIR" --target "$TARGET_NAME" -j

echo ""
echo "=== Launching RAD Debugger ==="
cd "$BUILD_DIR"
"$RADDBG_EXE" --auto_run "${TARGET_NAME}.exe"