from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout, CMakeToolchain, CMakeDeps


class ConanTestConan(ConanFile):
    name = "conan_test"
    version = "0.0.0"
    license = "MIT"
    author = "Tamino Bauknecht (dev@tb6.eu)"
    url = "https://github.com/taminob/ppplugin"

    settings = "os", "compiler", "build_type", "arch"
    generators = CMakeDeps, CMakeToolchain

    exports_sources = (
        "CMakeLists.txt",
        "a.cpp",
    )

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("ppplugin/0.0.0")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
