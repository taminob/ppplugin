#!/bin/sh

set -e
set -x

ROOT_SOURCE_DIR="${1}"
BUILD_DIR="${2}"
INSTALL_ROOT="${3}"
BUILD_SHARED="${4}"
BUILD_CPP17_COMPATIBLE="${5}"

echo "Building ppplugin library (shared: ${BUILD_SHARED})..."

echo "Use sources at '${ROOT_SOURCE_DIR}'..."
cmake "${ROOT_SOURCE_DIR}" -B "${BUILD_DIR}" -DPPPLUGIN_SHARED="${BUILD_SHARED}" -DPPPLUGIN_ENABLE_CPP17_COMPATIBILITY="${BUILD_CPP17_COMPATIBLE}"
echo "Build in '${BUILD_DIR}'..."
cmake --build "${BUILD_DIR}"
echo "Install to '${INSTALL_ROOT}'..."
cmake --install "${BUILD_DIR}" --prefix "${INSTALL_ROOT}/usr/local"
