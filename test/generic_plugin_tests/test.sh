#!/bin/sh

export string_global="abc"
export int_global=12
export float_global=42.0
export bool_global=0
export char_global="9"
export list_global='"" "1" "1 2"'


get_global(name) {
    eval "echo ${name}"
}


accept_int_string_bool_float(i, s, b, f) {
    if [ $# == 4 ]; then
        echo 1
    else
        echo 0
    fi
}


accept_dict() {
}


accept_list() {
    for x in ${1}; do
        echo "${x},"
    done
}


identity() {
    echo "${1}"
}
