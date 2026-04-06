from conan import ConanFile
from conan.tools.files import copy, get
import os

class ScvConan(ConanFile):
    name = "scv"
    version = "cci.20260404"
    description = "Small C Vector library"
    license = "ISC"
    url = "https://github.com/jibsen/scv"
    
    settings = "os", "arch", "compiler", "build_type"
    
    def package_id(self):
        # For cci versions without semantic versioning
        del self.info.settings.compiler.version
    
    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)
    
    def build(self):
        cc = os.environ.get("CC", "gcc")
        ar = os.environ.get("AR", "ar")
        
        cflags = ["-O2", "-fPIC", "-c"]
        if self.settings.build_type == "Debug":
            cflags = ["-g", "-fPIC", "-c"]
        
        src = os.path.join(self.source_folder, "scv.c")
        obj = os.path.join(self.build_folder, "scv.o")
        lib = os.path.join(self.build_folder, "libscv.a")
        
        self.run(f"{cc} {' '.join(cflags)} {src} -o {obj}")
        
        self.run(f"{ar} rcs {lib} {obj}")
    
    def package(self):
        copy(
            self,
            "scv.h",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "include"),
        )
        
        copy(
            self,
            "libscv.a",
            src=self.build_folder,
            dst=os.path.join(self.package_folder, "lib"),
        )
    
    def package_info(self):
        self.cpp_info.libs = ["scv"]
        self.cpp_info.set_property("cmake_file_name", "scv")
        self.cpp_info.set_property("cmake_target_name", "scv::scv")
