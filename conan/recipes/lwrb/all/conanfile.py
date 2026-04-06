from conan import ConanFile
from conan.tools.files import copy, get, replace_in_file
import os

class LwrbConan(ConanFile):
    name = "lwrb"
    version = "3.2.0"
    description = "Lightweight ring buffer manager"
    license = "MIT"
    url = "https://github.com/MaJerle/lwrb"
    
    settings = "os", "arch", "compiler", "build_type"
    
    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)
        
        # Apply patch: add LWRB_DISABLE_ATOMIC for old GCC compatibility
        header_path = os.path.join(
            self.source_folder, "lwrb", "src", "include", "lwrb", "lwrb.h"
        )
        replace_in_file(
            self,
            header_path,
            "#ifndef LWRB_HDR_H",
            "#ifndef LWRB_HDR_H\n#define LWRB_DISABLE_ATOMIC"
        )
    
    def build(self):
        cc = os.environ.get("CC", "gcc")
        ar = os.environ.get("AR", "ar")
        
        cflags = ["-O2", "-fPIC", "-std=c11"]
        if self.settings.build_type == "Debug":
            cflags = ["-g", "-fPIC", "-std=c11"]
        
        src = os.path.join(
            self.source_folder, "lwrb", "src", "lwrb", "lwrb.c"
        )
        obj = os.path.join(self.build_folder, "lwrb.o")
        lib = os.path.join(self.build_folder, "liblwrb.a")
        
        include_dir = os.path.join(self.source_folder, "lwrb", "src", "include")
        
        self.run(f"{cc} {' '.join(cflags)} -I {include_dir} -c {src} -o {obj}")
        
        self.run(f"{ar} rcs {lib} {obj}")
    
    def package(self):
        copy(
            self,
            "lwrb.h",
            src=os.path.join(self.source_folder, "lwrb", "src", "include", 
             "lwrb"),
            dst=os.path.join(self.package_folder, "include"),
        )
        
        copy(
            self,
            "liblwrb.a",
            src=self.build_folder,
            dst=os.path.join(self.package_folder, "lib"),
        )
    
    def package_info(self):
        self.cpp_info.libs = ["lwrb"]
        self.cpp_info.set_property("cmake_file_name", "lwrb")
        self.cpp_info.set_property("cmake_target_name", "lwrb::lwrb")
