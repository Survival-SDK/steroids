from conan import ConanFile
from conan.tools.files import copy, get, replace_in_file
import os

class HashTableConan(ConanFile):
    name = "hash_table"
    version = "cci.20260404"
    description = "Hash table implementation"
    license = "MIT"
    url = "https://github.com/anholt/hash_table"
    
    settings = "os", "arch", "compiler", "build_type"
    
    def package_id(self):
        del self.info.settings.compiler.version
    
    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)
        
        # Apply patches: change int to bool for key_equals_function
        replace_in_file(
            self,
            os.path.join(self.source_folder, "hash_table.c"),
            "int (*key_equals_function)",
            "bool (*key_equals_function)"
        )
        
        replace_in_file(
            self,
            os.path.join(self.source_folder, "hash_table.h"),
            "int (*key_equals_function)",
            "bool (*key_equals_function)"
        )
        
        # Add stdbool.h include after inttypes.h
        replace_in_file(
            self,
            os.path.join(self.source_folder, "hash_table.h"),
            "#include <inttypes.h>",
            "#include <inttypes.h>\n#include <stdbool.h>"
        )
    
    def build(self):
        # Simple compilation without build system        
        cc = os.environ.get("CC", "gcc")
        ar = os.environ.get("AR", "ar")
        
        cflags = ["-O2", "-c"]
        if self.settings.build_type == "Debug":
            cflags = ["-g", "-c"]
        
        src = os.path.join(self.source_folder, "hash_table.c")
        obj = os.path.join(self.build_folder, "hash_table.o")
        
        self.run(f"{cc} {' '.join(cflags)} {src} -o {obj}")
        
        lib = os.path.join(self.build_folder, "libhash_table.a")
        self.run(f"{ar} rcs {lib} {obj}")
    
    def package(self):
        copy(
            self,
            "hash_table.h",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "include"),
        )
        
        copy(
            self,
            "libhash_table.a",
            src=self.build_folder,
            dst=os.path.join(self.package_folder, "lib"),
        )
    
    def package_info(self):
        self.cpp_info.libs = ["hash_table"]
        self.cpp_info.set_property("cmake_file_name", "hash_table")
        self.cpp_info.set_property("cmake_target_name", "hash_table::hash_table")
