from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class TestCmakeBarebonesConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.tool_requires(self.tested_reference_str)

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()

    def test(self):
        # Just check that build succeeded
        pass
