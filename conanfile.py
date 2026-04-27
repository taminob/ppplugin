from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout, CMakeToolchain, CMakeDeps


class PppluginConan(ConanFile):
    name = "ppplugin"
    version = "0.0.0"
    description = "C++ Library for managing multi-language plugins to dynamically extend a compiled binary"
    license = "MIT"
    author = "Tamino Bauknecht (dev@tb6.eu)"
    url = "https://github.com/taminob/ppplugin"

    # TODO package_type? can be static-library or shared-library
    settings = "os", "compiler", "build_type", "arch"
    # generators = "CMakeDeps", "CMakeToolchain" # TODO: remove, done manually
    options = {
        "shared": [True, False],
    }
    default_options = {
        "shared": False,
    }

    exports_sources = (
        "CMakeLists.txt",
        "include/*",
        "src/*",
        "cmake/*",
        "test/*",
        "examples/*",
    )

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("boost/1.87.0")
        self.requires("cpython/3.10.0", options={"with_tkinter": False})
        self.requires("lua/5.4.7")

    def generate(self):
        tc = CMakeToolchain(self)

        tc.variables["PPPLUGIN_SHARED"] = self.options.shared

        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["ppplugin"]
        self.cpp_info.requires = [
            "cpython::cpython",
            "lua::lua",
            "boost::boost",
        ]
