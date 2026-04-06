from conan import ConanFile
from conan.tools.files import copy, get
import os

class CfgpathConan(ConanFile):
    name = "cfgpath"
    version = "cci.20260404"
    description = "Cross-platform C header library for obtaining paths to configuration files"
    license = "Unlicense"
    url = "https://github.com/Malvineous/cfgpath"
    topics = ("header-only", "config", "cross-platform")
    
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
            "cfgpath.h",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "include"),
        )
        copy(
            self,
            "LICENSE",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )
    
    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.set_property("cmake_file_name", "cfgpath")
        self.cpp_info.set_property("cmake_target_name", "cfgpath::cfgpath")
