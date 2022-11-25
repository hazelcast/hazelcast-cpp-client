set(GENERATE_DOCS_TARGET_ENABLE "TRUE")

if (WIN32)
    find_program(BASH_BIN bash.exe git-bash.exe)
    find_program(DOXYGEN_BIN doxygen.exe)

    if ("${BASH_BIN}" STREQUAL "BASH_BIN-NOTFOUND")
        message(
            "\nDisabling `generate-docs` target because could not found `git-bash.exe`.\n"
            "To make it worked install `git` so `bash.exe` comes with it.\n"
            "If `bash.exe` is already installed then add its folder to $PATH envrionment variable.\n"
            "It is generally located at 'C:\\Program Files\\Git\\bin'\n"
        )

        set(GENERATE_DOCS_TARGET_ENABLE "FALSE")
    endif()

    if ("${DOXYGEN_BIN}" STREQUAL "DOXYGEN_BIN-NOTFOUND")

        message(
            "\nDisabling `generate-docs` target because could not found `doxygen.exe`.\n"
            "To make it worked install `doxygen.exe`.\n"
            "If `doxygen.exe` is already installed then add its folder to $PATH envrionment variable.\n"
        )

        set(GENERATE_DOCS_TARGET_ENABLE "FALSE")
    endif()
endif()

if (${GENERATE_DOCS_TARGET_ENABLE})
    add_custom_target(
        generate-docs

        COMMENT
            "Generates doxygen documentations"
    )

    if (WIN32)
        add_custom_command(
            TARGET
                generate-docs
            WORKING_DIRECTORY
                ${CMAKE_SOURCE_DIR}
            COMMAND
                ${BASH_BIN}
                ${CMAKE_SOURCE_DIR}/scripts/generate-docs.sh
                "-m"
                    "README.md"
                "-v"
                    ${PROJECT_VERSION}
                "-u"
                    "https://github.com/hazelcast/hazelcast-cpp-client"
        )
    else()
        add_custom_command(
            TARGET
                generate-docs
            WORKING_DIRECTORY
                ${CMAKE_SOURCE_DIR}
            COMMAND
                ${CMAKE_SOURCE_DIR}/scripts/generate-docs.sh
                "-m"
                    "README.md"
                "-v"
                    ${PROJECT_VERSION}
                "-u"
                    "https://github.com/hazelcast/hazelcast-cpp-client"
        )
    endif()
endif()