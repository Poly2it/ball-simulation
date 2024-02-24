#!/bin/bash


# This program tries to place you in an environment suitable for building this 
# project on a best-effort basis.


export PROGRAM_NAME="$0"
export PROGRAM_VERSION="0.0.1"
export PROGRAM_AUTHOR="polybit"
export PROGRAM_LOCATION="$(dirname "$0")"
export PROGRAM_WORKING_DIRECTORY="$(dirname "$(readlink -f "${PROGRAM_LOCATION}")")"


main() {
    trap '' SIGINT

    if [ -n "${W64DEVKIT}" ]; then
        sh
    fi

    ENVIRONMENT="$(uname -o)"
    case "${ENVIRONMENT}" in
        "Msys")
            if [ ! -d "${PROGRAM_WORKING_DIRECTORY}/resources/external/w64devkit" ]; then
                msys_environment_download
            fi
            "${PROGRAM_WORKING_DIRECTORY}/resources/external/w64devkit/w64devkit.exe"
            ;;
        *)
            local PROGRAM_COUNT=0
            local PROGRAM_COUNT_TARGET=0

            if $(which sh           2>&1 1>/dev/null); then PROGRAM_COUNT+=1; fi; PROGRAM_COUNT_TARGET+=1
            if $(which make         2>&1 1>/dev/null); then PROGRAM_COUNT+=1; fi; PROGRAM_COUNT_TARGET+=1
            if $(which gcc          2>&1 1>/dev/null); then PROGRAM_COUNT+=1; fi; PROGRAM_COUNT_TARGET+=1
            if $(which git          2>&1 1>/dev/null); then PROGRAM_COUNT+=1; fi; PROGRAM_COUNT_TARGET+=1

            if [ ${PROGRAM_COUNT} -eq ${PROGRAM_COUNT_TARGET} ]; then
                sh
            else
                print "Unsuitable environment on unsupported platform"
                print "You may contact the authors for further assistance (${PROGRAM_AUTHOR})"
            fi
            ;;
    esac
}


msys_environment_download() {
    print "Requesting w64devkit release metadata"
    local GITHUB_RELEASE="$(curl -L \
        -H "Accept: application/vnd.github+json" \
        -H "X-GitHub-Api-Version: 2022-11-28" \
        https://api.github.com/repos/skeeto/w64devkit/releases/latest \
    )"
    local RELEASE_VERSION="$( \
        echo "${GITHUB_RELEASE}" \
            | python -c "import sys, json; print(json.load(sys.stdin)['name'])"
    )"
    local RELEASE_CONTENT="$( \
        echo "${GITHUB_RELEASE}" \
            | python -c "import sys, json; print(list(file.get('browser_download_url') for file in json.load(sys.stdin)['assets'] if file.get('name') == 'w64devkit-${RELEASE_VERSION}.zip')[0])" \
    )"
    local RELEASE_SIGNATURE="$( \
        echo "${GITHUB_RELEASE}" \
            | python -c "import sys, json; print(list(file.get('browser_download_url') for file in json.load(sys.stdin)['assets'] if file.get('name') == 'w64devkit-${RELEASE_VERSION}.zip.sig')[0])" \
    )"
    print "${RELEASE_SIGNATURE}"

    print "Creating temporary extraction directory"
    local PATH_EXTRACT_DIRECTORY="$(mktemp -d)"
    trap "rm -rf \"${PATH_EXTRACT_DIRECTORY}\"; print 'Error on line $(caller), cleaning temporary directories'; exit 1" ERR
    
    print "Requesting release"
    curl "${RELEASE_CONTENT}" -L \
        -H "Accept: application/vnd.github+json" \
        -H "X-GitHub-Api-Version: 2022-11-28" \
        --output "${PATH_EXTRACT_DIRECTORY}/w64devkit.zip"

    print "Requesting signature"
    curl "${RELEASE_SIGNATURE}" -L \
        -H "Accept: application/vnd.github+json" \
        -H "X-GitHub-Api-Version: 2022-11-28" \
        --output "${PATH_EXTRACT_DIRECTORY}/w64devkit.zip.sig"

    print "Importing gpg key"
    gpg --import "${PROGRAM_WORKING_DIRECTORY}/resources/keys/com.nullprogram.wellons.key" 2>/dev/null
    print "Verifying files"
    if $(
        gpg --verify \
        "${PATH_EXTRACT_DIRECTORY}/w64devkit.zip.sig" \
        "${PATH_EXTRACT_DIRECTORY}/w64devkit.zip" \
        2>&1 \
        | grep --quiet "gpg: Good signature from \"Christopher Wellons <wellons@nullprogram.com>\""
    ); then
        print "Good signature"
    else
        print "Bad signature, please contact the author (${PROGRAM_AUTHOR})"
        exit 1
    fi

    print "Extracting"
    cd "${PATH_EXTRACT_DIRECTORY}"
    unzip "${PATH_EXTRACT_DIRECTORY}/w64devkit.zip" \
        | awk \
            -v width=$(tput cols) \
            '{ printf "\33[2K\r%s", substr($0, 3, width) } END { printf "\n" }'
    
    mv "${PATH_EXTRACT_DIRECTORY}/w64devkit" "${PROGRAM_WORKING_DIRECTORY}/resources/external"
    
    cd "${PROGRAM_WORKING_DIRECTORY}"
    rm -rf "${PATH_EXTRACT_DIRECTORY}"
    trap -- ERR
}


print() {
    echo "$@" 1>&3
}


main "$@" 3>&1 4>&2
