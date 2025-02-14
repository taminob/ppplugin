#!/bin/sh

set -e
set -x

ROOT_SOURCE_DIR="${1}"
BUILD_DIR="${2}"
INSTALL_ROOT="${3}"

if [ "${4}" = "SHARED" ]; then
    TEST_SHARED=ON
elif [ "${4}" = "STATIC" ]; then
    TEST_SHARED=OFF
else
    echo "Fourth argument must be 'SHARED' or 'STATIC'"
    return
fi

if [ "${5}" = "Cpp17" ]; then
    TEST_CPP17_COMPATIBILITY=ON
elif [ "${5}" = "Cpp20" ]; then
    TEST_CPP17_COMPATIBILITY=OFF
else
    echo "Fifth argument must be 'Cpp17' or 'Cpp20'"
    return
fi

TEST_BUILD_DIR="${BUILD_DIR}/shared_${TEST_SHARED}_cpp17_${TEST_CPP17_COMPATIBILITY}"
EXECUTABLE_NAME="${TEST_BUILD_DIR}/a"

"${ROOT_SOURCE_DIR}/test/cmake_tests/build_and_install_ppplugin.sh" "${ROOT_SOURCE_DIR}" "${BUILD_DIR}/ppplugin_build_shared_${TEST_SHARED}" "${INSTALL_ROOT}" "${TEST_SHARED}"

CMAKE_PREFIX_PATH="${INSTALL_ROOT}/usr/local/lib/cmake" \
    cmake . --log-level DEBUG -B "${TEST_BUILD_DIR}" -DPPPLUGIN_SHARED="${TEST_SHARED}" -DPPPLUGIN_ENABLE_CPP17_COMPATIBILITY="${TEST_CPP17_COMPATIBILITY}"
cmake --build "${TEST_BUILD_DIR}"

"${EXECUTABLE_NAME}"
