#!/bin/sh


PROGRAM_NAME="$0"
PROGRAM_VERSION="0.0.1"
PROGRAM_AUTHOR="polybit"
PROGRAM_LOCATION="$(dirname "$0")"
PROGRAM_WORKING_DIRECTORY="$(dirname "$(readlink -f "${PROGRAM_LOCATION}")")"


case "$(uname -o)" in
    "Msys")
        PROGRAM_WORKING_DIRECTORY_W64DEVKIT="$( \
            echo ${PROGRAM_WORKING_DIRECTORY} \
                | awk '
                    {
                        printf "%s", toupper(substr($0, 2, 1))
                        printf ":"
                        printf "%s", substr($0, 3) 
                    } 
                    END { 
                        printf "\n" 
                    }
                ' \
        )"
        echo "
            cd \"${PROGRAM_WORKING_DIRECTORY_W64DEVKIT}\" && exec \"scripts/dependencies.sh\" $@
        " | "${PROGRAM_LOCATION}/environment.sh"
        exit 0
        ;;
    "MS/Windows")
        . "${PROGRAM_WORKING_DIRECTORY}/scripts/lib_w64devkit.sh"
        ;;
    "Linux" | "GNU/Linux")
        . "${PROGRAM_WORKING_DIRECTORY}/scripts/lib_linux.sh"
        ;;
    *)
        break
        ;;
esac


main() {
    local DEPENDENCY="$1"
    local SUBCOMMAND="$2"
    cd "${PROGRAM_WORKING_DIRECTORY}/resources/external"
    "${DEPENDENCY}_${SUBCOMMAND}" || printf "invalid dependency or subcommand: \"%s\" \"%s\"\n" "${DEPENDENCY}" "${SUBCOMMAND}"
}


git_require_commit() {
    local HOST="$1"
    local PATH_TARGET="$2"
    local HASH_REQUIRED="$3"
    local NO_SHALLOW="$4"

    if [ ! -d "${PATH_TARGET}" ]; then
        if "${NO_SHALLOW}"; then
            git clone "${HOST}" "${PATH_TARGET}"
        else
            git clone "${HOST}" "${PATH_TARGET}" --depth=1
        fi
    fi

    (
        cd "${PATH_TARGET}"

        HASH_LOCAL="$( \
            git log \
                --max-count=1 \
                --pretty=format:"%H"
        )"

        if [ "${HASH_LOCAL}" = "${HASH_REQUIRED}" ]; then
            echo "${PATH_TARGET}: Up to date"
        else
            echo "${PATH_TARGET}: Updating"
            git pull 2>&1 1>&3 || exit 1
            git checkout "${HASH_REQUIRED}" 2>&1 1>&3 || exit 1
            echo "${PATH_TARGET}: Finished updating, up to date"
        fi
    )
}


raylib_require() {
    git_require_commit \
        "https://github.com/raysan5/raylib.git" \
        "raylib" \
        "deaffb06981b95976f7b055dc1085adb933a37cc" \
        "false"
}


raylib_build() {
    (
        cd "raylib/src"
        HASH_LOCAL="$( \
            git log \
                --max-count=1 \
                --pretty=format:"%H"
        )"
        if [ ! "$(cat BUILD_HASH)" = "${HASH_LOCAL}" ]; then
            make -j$(nproc) 1>&3 2>&4
            echo "${HASH_LOCAL}" > "BUILD_HASH"
        fi
    )
}


raylib_install() {
    (
        cd "raylib/src"

        mkdir --parents "$(path_local_share_get)/lib"
        mkdir --parents "$(path_local_share_get)/include"

        cp --update --verbose libraylib.a "$(path_local_share_get)/lib/libraylib.a"
        cp --update --verbose raylib.h "$(path_local_share_get)/include/raylib.h"
        cp --update --verbose raymath.h "$(path_local_share_get)/include/raymath.h"
        cp --update --verbose rlgl.h "$(path_local_share_get)/include/rlgl.h"
    )
}


raylib_uinstall() {
    rm "$(path_local_share_get)/lib/libraylib.a"
    rm "$(path_local_share_get)/include/raylib.h"
    rm "$(path_local_share_get)/include/raymath.h"
    rm "$(path_local_share_get)/include/rlgl.h"
}


# raylib_all is an amalgamate of the above scripts, performing their actions 
# faster assuming they would be run in succession. It should be considered
# less stable than its parent functions.
raylib_all() {
    local HOST="https://github.com/raysan5/raylib.git"
    local PATH_TARGET="raylib"
    local HASH_REQUIRED="29ff658d9223068378269e4a705811f085fafdf4"

    if [ ! -d "${PATH_TARGET}" ]; then
        git clone "${HOST}" "${PATH_TARGET}" 2>&1 1>&3 || exit 1

        HASH_LOCAL="$( \
            git log \
                --max-count=1 \
                --pretty=format:"%H"
        )"

        if [ "${HASH_LOCAL}" = "${HASH_REQUIRED}" ]; then
            git checkout "${HASH_REQUIRED}" 2>&1 1>&3 || exit 1
        fi

        (
            cd "raylib/src"
            make -j$(nproc) 1>&3 2>&4

            mkdir --parents "$(path_local_share_get)/lib"
            mkdir --parents "$(path_local_share_get)/include"

            cp --update libraylib.a "$(path_local_share_get)/lib/libraylib.a"
            cp --update raylib.h "$(path_local_share_get)/include/raylib.h"
            cp --update raymath.h "$(path_local_share_get)/include/raymath.h"
            cp --update rlgl.h "$(path_local_share_get)/include/rlgl.h"
        )
    else
        (
            cd "${PATH_TARGET}"

            HASH_LOCAL="$( \
                git log \
                    --max-count=1 \
                    --pretty=format:"%H"
            )"

            if [ "${HASH_LOCAL}" = "${HASH_REQUIRED}" ]; then
                echo "${PATH_TARGET}: Up to date"
            else
                echo "${PATH_TARGET}: Updating"
                git fetch 2>&1 1>&3 || exit 1
                git checkout "${HASH_REQUIRED}" 2>&1 1>&3 || exit 1

                make -j$(nproc) 1>&3 2>&4

                (
                    cd "src"
                    
                    mkdir --parents "$(path_local_share_get)/lib"
                    mkdir --parents "$(path_local_share_get)/include"

                    cp --update --verbose libraylib.a "$(path_local_share_get)/lib/libraylib.a"
                    cp --update --verbose raylib.h "$(path_local_share_get)/include/raylib.h"
                    cp --update --verbose raymath.h "$(path_local_share_get)/include/raymath.h"
                    cp --update --verbose rlgl.h "$(path_local_share_get)/include/rlgl.h"
                )
                echo "${PATH_TARGET}: Finished updating, up to date"
            fi
        )
    fi
}


main "$@" 3>&1 4>&2
