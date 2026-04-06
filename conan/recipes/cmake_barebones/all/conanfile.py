from conan import ConanFile
from conan.tools.files import copy, get, load, save
import os
import re

class CmakeBarebonesConan(ConanFile):
    name = "cmake_barebones"
    version = "cci.20260404"
    description = "Common CMake code and utilities"
    license = "CC0-1.0"
    url = "https://github.com/Survival-SDK/cmake_barebones"
    topics = ("cmake", "build-scripts", "utilities")
    
    settings = "os", "arch", "compiler", "build_type"
    package_type = "build-scripts"
    no_copy_source = True
    
    def package_id(self):
        self.info.clear()
    
    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)
    
    def build(self):
        pass
    
    def package(self):
        # Copy CMake modules
        copy(
            self,
            "*.cmake",
            src=os.path.join(self.source_folder, "cmake", "barebones"),
            dst=os.path.join(self.package_folder, "lib", "cmake", "barebones"),
        )
        
        # Process and copy Make include file
        makefile_in = os.path.join(
            self.source_folder, "makefiles", "barebones.mk.in"
        )
        makefile_content = load(self, makefile_in)
        
        # Replace @BB_PREFIX@ with package folder path
        makefile_content = re.sub(
            r'@BB_PREFIX@',
            self.package_folder.replace('\\', '/'),
            makefile_content
        )
        
        makefile_out = os.path.join(
            self.package_folder, "include", "barebones.mk"
        )
        save(self, makefile_out, makefile_content)
        
        # Copy license
        copy(
            self,
            "COPYING",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )
    
    def package_info(self):
        # Add CMake modules to CMAKE_MODULE_PATH
        self.cpp_info.builddirs = [os.path.join("lib", "cmake")]
        
        # Add Make include directory
        self.cpp_info.includedirs = ["include"]
        
        # No libraries to link
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        
        self.cpp_info.set_property("cmake_file_name", "barebones")
        self.cpp_info.set_property("cmake_target_name", "barebones::barebones")
