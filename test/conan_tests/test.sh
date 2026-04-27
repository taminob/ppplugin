#!/bin/sh

set -e
set -x

ROOT_SOURCE_DIR="${1}"
TEST_DIR="${2}"

EXECUTABLE_NAME="${TEST_DIR}/build/Release/a"

conan create "${ROOT_SOURCE_DIR}"
conan build "${TEST_DIR}" --build=missing

"${EXECUTABLE_NAME}"
