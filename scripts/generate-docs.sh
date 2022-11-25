#!/usr/bin/env bash

set -e
usage() { echo "Usage: $(basename $0) -m main_page_path -v project_version -u repo_url"; exit $1; }

# Parse args
while getopts 'm:v:u:h' opt; do
    case "$opt" in
        m) MAINPAGE=${OPTARG}           ;;
        v) PROJECT_VERSION="v${OPTARG}" ;;
        u) REPO_URL=${OPTARG}           ;;
        h) usage "0"                    ;;
    esac
done

# Check mandatory options
if [ -z "${MAINPAGE}" ]; then
    echo "Missing -m main_page_path" >&2
    usage "1"
fi

if [ -z "${PROJECT_VERSION}" ]; then
    echo "Missing -v project_version" >&2
    usage "1"
fi

if [ -z "${REPO_URL}" ]; then
    echo "Missing -u repo_url" >&2
    usage "1"
fi

BASE_URL=${REPO_URL}/blob/${PROJECT_VERSION}
GREP_BIN="grep"

if [[ "$OSTYPE" == "darwin"* ]]; then
    GREP_BIN="ggrep"

    if [ ! -x "${GREP_BIN} -v" ]; then
        echo "===================================="
        echo "${GREP_BIN} tool is not installed."
        echo "Install it with 'brew install grep'"
        echo "===================================="
        exit 1
    fi
fi

###################################################
# Currently Doxygen cannot parse relative links   #
# which are located in the markdown files.        #
# Example :                                       #
# [Conan](Reference_Manual.md#111-conan-users)    #
# The line above is a relative link to the section#
# which is located another file. So this part of  #
# script converts these links into absolute ones  #
#                                                 #
# For example :                                   #
# [Conan](Reference_Manual.md#111-conan-users)    #
# becomes                                         #
# [Conan](${REPO_URL}/blob/${PROJECT_VERSION})    #
# so it substitutes                               #
# [Conan](https://github.com/hazelcast/hazelcast-cpp-client/blob/v5.0.0/Reference_Manual.md#111-conan-users) #
###################################################

FIND_RELATIVE_LINKS_REGEXP='\[((\w|\d|\s)+)\]\((?!http)((\w|\d|)+)(\.((\w)+))?(\#((\w|\d|\-)+))?\)'

IFS=$'\n'
RELATIVE_LINKS=( $(LC_ALL=en_US.utf8 ${GREP_BIN} -Po ${FIND_RELATIVE_LINKS_REGEXP} ${MAINPAGE}) )

ARR=( $(printf "%s\n%s\n" "1" "ozan cansel 2") )

declare -p ARR

FORMATTED=formatted.${MAINPAGE}
cp ${MAINPAGE} ${FORMATTED}

# Replace relative cross-links with the absolute ones
for (( i=0; i<${#RELATIVE_LINKS[@]}; i++ ));
do
    RELATIVE_LINK=${RELATIVE_LINKS[$i]}

    if [[ "${RELATIVE_LINK}" =~  (.+\()(.+)\) ]]
    then
        URL=${BASE_URL}/${BASH_REMATCH[2]}
        REPLACED_LINK="${BASH_REMATCH[1]}${URL})"
    fi

    RELATIVE_LINK=$(echo ${RELATIVE_LINK} | sed "s/\[/\\\[/g")
    RELATIVE_LINK=$(echo ${RELATIVE_LINK} | sed "s/\]/\\\]/g")

    REPLACED_LINK=$(echo ${REPLACED_LINK} | sed "s/\[/\\\[/g")
    REPLACED_LINK=$(echo ${REPLACED_LINK} | sed "s/\]/\\\]/g")
    REPLACED_LINK=$(echo ${REPLACED_LINK} | sed 's/\//\\\//g')

    SED_EXPRESSION="s/${RELATIVE_LINK}/${REPLACED_LINK}/g"

    sed -i "${SED_EXPRESSION}" ${FORMATTED}
done

doxygen

rm ${FORMATTED}
