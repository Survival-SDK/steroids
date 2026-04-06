from conan import ConanFile
from conan.tools.files import copy, get
import os

class KetoptConan(ConanFile):
    name = "ketopt"
    version = "cci.20260404"
    description = "Portable command-line option parser (from klib)"
    license = "MIT"
    url = "https://github.com/attractivechaos/klib"
    topics = ("header-only", "command-line", "option-parser")
    
    settings = "os", "arch", "compiler", "build_type"
    no_copy_source = True
    
    def package_id(self):
        self.info.clear()
    
    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)
    
    def build(self):
        pass
    
    def package(self):
        copy(
            self,
            "ketopt.h",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "include", "klib"),
        )
    
    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.set_property("cmake_file_name", "ketopt")
        self.cpp_info.set_property("cmake_target_name", "ketopt::ketopt")
