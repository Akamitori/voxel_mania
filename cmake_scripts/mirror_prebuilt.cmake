# cmake -DSRC=<cache prefix> -DDST=<build-dir copy> -DSTAMP=<file> -P mirror_prebuilt.cmake
#
# Mirrors <SRC>/lib and <SRC>/bin (top-level files only) into <DST>, which lives in the build dir.
# Imported targets point at the copy, so CMake's Clean / Rebuild only deletes the copy and never
# files in the shared .prebuild_libs cache.
#
# - Regular files are copied (fresh timestamps, so Ninja sees them as newer than their inputs).
# - Symlinks (libfoo.so -> libfoo.so.1 -> libfoo.so.1.2.3) are recreated as symlinks to the
#   copied file, so the runtime soname still resolves and the library is stored only once.
# - Subdirectories (cmake/, pkgconfig/) are skipped: nothing links against them.

foreach (var SRC DST STAMP)
    if (NOT DEFINED ${var})
        message(FATAL_ERROR "mirror_prebuilt.cmake: ${var} not set")
    endif ()
endforeach ()

file(REMOVE_RECURSE "${DST}")

foreach (sub lib bin)
    file(MAKE_DIRECTORY "${DST}/${sub}")
    file(GLOB entries LIST_DIRECTORIES true RELATIVE "${SRC}/${sub}" "${SRC}/${sub}/*")
    foreach (entry IN LISTS entries)
        set(from "${SRC}/${sub}/${entry}")
        set(to "${DST}/${sub}/${entry}")
        if (IS_SYMLINK "${from}")
            file(READ_SYMLINK "${from}" target)
            # Keep links inside the copy: point at the sibling with the same file name
            get_filename_component(target "${target}" NAME)
            file(CREATE_LINK "${target}" "${to}" SYMBOLIC)
        elseif (NOT IS_DIRECTORY "${from}")
            file(COPY_FILE "${from}" "${to}")
        endif ()
    endforeach ()
endforeach ()

file(TOUCH "${STAMP}")
