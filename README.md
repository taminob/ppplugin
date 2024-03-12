# PPPlugin

[!WARNING]
Until version 1.0.0 is released, all interfaces and features are subject
to change and there are no guarantees regarding stability.

This library aims to provide an easy-to-use and modern C++ interface
to extend an existing library at runtime via plugins.

These plugins can be written in C++, C, Lua and Python.
All languages share, once loaded, the same interface and can be used
interchangeably.

## Usage

```cmake
# set(PPPLUGIN_SHARED ON) # uncomment to use shared instead of static library
find_package(ppplugin REQUIRED)

target_link_library(<target> ppplugin::ppplugin)
```

For code usage, please refer to the different examples in `examples/`.

## Compilation

To build the repository with default options, run the following commands:

```console
cmake . -B build
cmake --build build -j
cmake --install build --prefix build/root/usr/local
```

This will compile and install the library to the directory `build/root`.

Available compilation options:

| Name                       |Default| Description                              |
| -------------------------- | ----- | ---------------------------------------- |
| `PPPLUGIN_SHARED`          | `OFF` | Compile as shared library                |
| `PPPLUGIN_ENABLE_COVERAGE` | `OFF` | Enable flags for measuring test coverage |
| `PPPLUGIN_ENABLE_TESTS`    | `OFF` | Tests in `tests` will be compiled        |
| `PPPLUGIN_ENABLE_EXAMPLES` | `OFF` | Examples in `examples` will be compiled  |
| `DPPPLUGIN_ENABLE_CPP17_COMPATIBILITY` | `OFF` |                              |
||| Library will be compiled with C++17 compatibility                           |

Extend the first command above with the desired options, for example:

```console
cmake . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPPPLUGIN_SHARED=ON \
  -DPPPLUGIN_ENABLE_EXAMPLES=ON \
  -DPPPLUGIN_ENABLE_TESTS=ON \
  -DPPPLUGIN_ENABLE_CPP17_COMPATIBILITY=ON
```
