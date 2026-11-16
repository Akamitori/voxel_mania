# cmake -DPREBUILT_DIR=<.prebuild_libs> -DKEEP_COMMITS=<c1,c2,...> -P prune_prebuilt_libs.cmake
#
# Deletes cache dirs under .prebuild_libs/<lib>/ that were built from a submodule commit that no
# library uses any more (KEEP_COMMITS = the current commits of all import_external_library
# sources). Such dirs can never be reused. Dirs for other compilers, build types or linkers at the
# current commits are kept, since other build dirs may be using them. Incomplete dirs (no
# .build_fingerprint, e.g. a build in progress) are left alone.

if (NOT DEFINED PREBUILT_DIR OR NOT DEFINED KEEP_COMMITS)
    message(FATAL_ERROR "prune_prebuilt_libs.cmake: PREBUILT_DIR and KEEP_COMMITS must be set")
endif ()
string(REPLACE "," ";" KEEP_COMMITS "${KEEP_COMMITS}")

file(GLOB fingerprints "${PREBUILT_DIR}/*/*/.build_fingerprint")
set(removed 0)
foreach (fp IN LISTS fingerprints)
    get_filename_component(cache_dir "${fp}" DIRECTORY)
    file(STRINGS "${fp}" commit_line REGEX "^commit=" LIMIT_COUNT 1)
    string(REGEX REPLACE "^commit=" "" commit "${commit_line}")
    if (commit STREQUAL "nogit" OR commit IN_LIST KEEP_COMMITS)
        continue()
    endif ()
    file(RELATIVE_PATH rel "${PREBUILT_DIR}" "${cache_dir}")
    message(STATUS "Removing stale cache .prebuild_libs/${rel} (built from ${commit})")
    file(REMOVE_RECURSE "${cache_dir}")
    math(EXPR removed "${removed} + 1")
endforeach ()

# Drop <lib> folders that are now empty
file(GLOB lib_dirs LIST_DIRECTORIES true "${PREBUILT_DIR}/*")
foreach (lib_dir IN LISTS lib_dirs)
    if (IS_DIRECTORY "${lib_dir}")
        file(GLOB content "${lib_dir}/*")
        if (NOT content)
            file(REMOVE_RECURSE "${lib_dir}")
        endif ()
    endif ()
endforeach ()

message(STATUS "prune_prebuilt_libs: removed ${removed} stale cache dir(s)")
