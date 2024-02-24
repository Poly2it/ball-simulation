path_local_share_get() {
    # We can always expect this variable to be set as the root project directory
    echo "${PROGRAM_WORKING_DIRECTORY}/resources/share"
}


file_copy() {
    COPY_SOURCE_PATH="$1"
    COPY_DESTINATION_PATH="$2"
    cp "${COPY_SOURCE_PATH}" "${COPY_DESTINATION_PATH}"
}


file_copy_verbose() {
    COPY_SOURCE_PATH="$1"
    COPY_DESTINATION_PATH="$2"
    cp "${COPY_SOURCE_PATH}" "${COPY_DESTINATION_PATH}" \
        && echo "${COPY_SOURCE_PATH} -> ${COPY_DESTINATION_PATH}"
}


file_destroy() {
    REMOVE_PATH="$1"
    rm "${REMOVE_PATH}"
}


directory_copy() {
    COPY_SOURCE_PATH="$1"
    COPY_DESTINATION_PATH="$2"
    cp --recursive "${COPY_SOURCE_PATH}" "${COPY_DESTINATION_PATH}"
}


directory_copy_verbose() {
    COPY_SOURCE_PATH="$1"
    COPY_DESTINATION_PATH="$2"
    cp --recursive "${COPY_SOURCE_PATH}" "${COPY_DESTINATION_PATH}" \
        || echo "${COPY_SOURCE_PATH} -x ${COPY_DESTINATION_PATH}" \
        && echo "${COPY_SOURCE_PATH} -> ${COPY_DESTINATION_PATH}"
}


directory_destroy() {
    REMOVE_PATH="$1"
    rm --recursive --force "${REMOVE_PATH}"
}


directory_create() {
    DIRECTORY_CREATION_PATH="$1"
    mkdir "${DIRECTORY_CREATION_PATH}"
}


directory_create_recursive() {
    DIRECTORY_CREATION_PATH="$1"
    mkdir --parents "${DIRECTORY_CREATION_PATH}"
}

