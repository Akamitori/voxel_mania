# voxel_sync_submodules(): keep git submodules in step with the superproject automatically,
# without undoing submodules you checked out at another commit by hand.
#
# Per submodule (from .gitmodules), on every configure:
# - not initialized (fresh clone)                     -> git submodule update --init
# - recorded commit changed since the last sync (pull, branch switch) and the checkout is
#   still at the previously synced commit             -> git submodule update (follows the pointer)
# - recorded commit changed, but the checkout was moved by hand -> warning, left alone
# - recorded commit unchanged                         -> left alone (manual checkouts survive)
#
# "Recorded" = the commit in the superproject's index (what `git submodule update` uses).
# State (last synced recorded commit per submodule) lives in STATE_FILE, shared by all build dirs.
# -DVOXEL_SYNC_SUBMODULES=ON forces `git submodule update --init --recursive` instead.

include_guard(GLOBAL)

function(voxel_sync_submodules)
    cmake_parse_arguments(ARG "" "REPO_DIR;STATE_FILE" "" ${ARGN})
    find_package(Git REQUIRED)

    if (NOT EXISTS "${ARG_REPO_DIR}/.git")
        message(STATUS "Not a git checkout, skipping submodule sync")
        return()
    endif ()

    if (VOXEL_SYNC_SUBMODULES)
        message(STATUS "Submodules: forced sync (VOXEL_SYNC_SUBMODULES=ON)")
        execute_process(COMMAND "${GIT_EXECUTABLE}" submodule update --init --recursive
                WORKING_DIRECTORY "${ARG_REPO_DIR}" RESULT_VARIABLE rc)
        if (NOT rc EQUAL 0)
            message(FATAL_ERROR "git submodule update failed")
        endif ()
    endif ()

    execute_process(COMMAND "${GIT_EXECUTABLE}" config -f .gitmodules --get-regexp "^submodule\\..*\\.path$"
            WORKING_DIRECTORY "${ARG_REPO_DIR}" OUTPUT_VARIABLE path_lines OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE "\n" ";" path_lines "${path_lines}")

    set(state "")
    if (EXISTS "${ARG_STATE_FILE}")
        file(STRINGS "${ARG_STATE_FILE}" state)
    endif ()
    set(new_state "")

    foreach (line IN LISTS path_lines)
        string(REGEX REPLACE "^[^ ]+ " "" path "${line}")

        # Recorded commit (index)
        execute_process(COMMAND "${GIT_EXECUTABLE}" ls-files -s -- "${path}"
                WORKING_DIRECTORY "${ARG_REPO_DIR}" OUTPUT_VARIABLE ls OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REGEX MATCH "^160000 ([0-9a-f]+)" _ "${ls}")
        set(recorded "${CMAKE_MATCH_1}")
        if (NOT recorded)
            continue()
        endif ()

        # Last synced recorded commit
        set(last "")
        foreach (entry IN LISTS state)
            if (entry MATCHES "^${path} ([0-9a-f]+)$")
                set(last "${CMAKE_MATCH_1}")
            endif ()
        endforeach ()

        # Current checkout (empty if not initialized)
        set(current "")
        if (EXISTS "${ARG_REPO_DIR}/${path}/.git")
            execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
                    WORKING_DIRECTORY "${ARG_REPO_DIR}/${path}" OUTPUT_VARIABLE current
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        endif ()

        string(SUBSTRING "${recorded}" 0 7 recorded_short)
        set(do_update FALSE)
        if (NOT current)
            message(STATUS "Submodule ${path}: not initialized, initializing at ${recorded_short}")
            set(do_update TRUE)
        elseif (current STREQUAL recorded)
            # In sync
        elseif (NOT recorded STREQUAL last AND current STREQUAL last)
            message(STATUS "Submodule ${path}: recorded commit changed, updating to ${recorded_short}")
            set(do_update TRUE)
        elseif (NOT recorded STREQUAL last)
            string(SUBSTRING "${current}" 0 7 current_short)
            message(WARNING "Submodule ${path}: the superproject now records ${recorded_short}, but the "
                    "checkout is at ${current_short}, not the previously synced commit, so it was moved by "
                    "hand. Leaving it alone. To sync: git submodule update -- ${path}")
        endif ()
        # recorded == last and current != recorded: moved by hand, nothing changed upstream -> leave it

        if (do_update)
            execute_process(COMMAND "${GIT_EXECUTABLE}" submodule update --init --recursive -- "${path}"
                    WORKING_DIRECTORY "${ARG_REPO_DIR}" RESULT_VARIABLE rc)
            if (NOT rc EQUAL 0)
                message(FATAL_ERROR "git submodule update -- ${path} failed")
            endif ()
        endif ()
        list(APPEND new_state "${path} ${recorded}")
    endforeach ()

    list(JOIN new_state "\n" new_state)
    get_filename_component(state_dir "${ARG_STATE_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${state_dir}")
    file(WRITE "${ARG_STATE_FILE}" "${new_state}\n")
endfunction()
