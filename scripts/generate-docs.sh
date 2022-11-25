#!/usr/bin/env bash

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

CROSS_LINKS=()

# Find and fill cross-links
i=0
while read -r line; do
    CROSS_LINKS[${i}]=${line}

    i=$((${i}+1))
done < <(LC_ALL=en_US.utf8 grep -Po '\[((\w|\d|\s)+)\]\((?!http)((\w|\d|)+)(\.((\w)+))?(\#((\w|\d|\-)+))?\)' ${MAINPAGE})

FORMATTED=formatted.${MAINPAGE}
cp ${MAINPAGE} ${FORMATTED}

# Replace relative cross-links with the absolute ones
for (( i=0; i<${#CROSS_LINKS[@]}; i++ ));
do
    CROSS_LINK=${CROSS_LINKS[$i]}

    if [[ "${CROSS_LINK}" =~  (.+\()(.+)\) ]]
    then
        URL=${BASE_URL}/${BASH_REMATCH[2]}
        REPLACED_LINK="${BASH_REMATCH[1]}${URL})"
    fi

    CROSS_LINK=$(echo ${CROSS_LINK} | sed "s/\[/\\\[/g")
    CROSS_LINK=$(echo ${CROSS_LINK} | sed "s/\]/\\\]/g")

    REPLACED_LINK=$(echo ${REPLACED_LINK} | sed "s/\[/\\\[/g")
    REPLACED_LINK=$(echo ${REPLACED_LINK} | sed "s/\]/\\\]/g")
    REPLACED_LINK=$(echo ${REPLACED_LINK} | sed 's/\//\\\//g')

    SED_EXPRESSION="s/${CROSS_LINK}/${REPLACED_LINK}/g"

    sed -i "${SED_EXPRESSION}" ${FORMATTED}
done

doxygen

rm ${FORMATTED}
