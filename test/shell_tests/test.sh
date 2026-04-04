#!/bin/sh

export env_var="abc"

echo "source"

print_first()
{
    echo a > /tmp/a
    echo "${1}"
}

print_env_var()
{
    var_name="${1}"
    eval echo "\${${var_name}}"
}

change_environment()
{
    env_var="xyz"
}

exit_shell()
{
    exit
}
