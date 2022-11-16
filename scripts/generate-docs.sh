#!/bin/bash

MAINPAGE=${1}
PROJECT_VERSION="v${2}"
REPO_URL=${3}
BASE_URL=${REPO_URL}/blob/${PROJECT_VERSION}

readarray -t CROSS_LINKS < <(grep -Po '\[((\w|\d|\s)+)\]\((?!http)((\w|\d|)+)(\.((\w)+))?(\#((\w|\d|\-)+))?\)' ${MAINPAGE})

FORMATTED=formatted.${MAINPAGE}
cp ${MAINPAGE} ${FORMATTED}

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
