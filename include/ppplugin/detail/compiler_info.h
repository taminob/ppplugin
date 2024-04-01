#ifndef PPPLUGIN_DETAIL_COMPILER_INFO_H
#define PPPLUGIN_DETAIL_COMPILER_INFO_H

#include <string_view>
#include <tuple>

constexpr std::string_view TRUE_CXX =
#ifdef __clang__
    "clang++";
#elif defined(__GNUC__)
    "g++";
#else
    "?";
#endif
#include <string>

constexpr std::tuple<int, int, int> TRUE_CXX_VER =
#ifdef __clang__
    { __clang_major__, __clang_minor__, __clang_patchlevel__ };
#elif defined(__GNUC__)
    { __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__ };
#elif defined(_MSC_VER)
    { _MSC_VER, 0, 0 };
#else
#error "Unable to detect compiler version for ABI checking." \
        "Please consider disabling this feature via CMake."
#endif // __clang__

#endif // PPPLUGIN_DETAIL_COMPILER_INFO_H
