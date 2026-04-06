from conan import ConanFile
from conan.tools.files import get
import os

class LibsirConan(ConanFile):
    name = "libsir"
    version = "2.2.5"
    description = "Standard Incident Reporter - cross-platform logging library"
    license = "MIT"
    url = "https://github.com/aremmell/libsir"
    
    settings = "os", "arch", "compiler", "build_type"
    
    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)
    
    def build(self):
        make_args = [
            f"-j{os.cpu_count()}",
            "static"
        ]
        
        if self.settings.os == "Windows":
            make_args.extend([
                f"CC={os.environ.get('CC', 'gcc')}",
                f"AR={os.environ.get('AR', 'ar')}"
            ])
        
        self.run(f"make {' '.join(make_args)}", cwd=self.source_folder)
    
    def package(self):
        self.run(
            f"make install PREFIX={self.package_folder}",
            cwd=self.source_folder
        )
    
    def package_info(self):
        # Library name is libsir_s.a for static
        self.cpp_info.libs = ["sir_s"]
        self.cpp_info.set_property("cmake_file_name", "libsir")
        self.cpp_info.set_property("cmake_target_name", "libsir::libsir")
