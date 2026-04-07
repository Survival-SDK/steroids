from conan import ConanFile
from conan.tools.files import copy, get
import os


class EzxmlConan(ConanFile):
    name = "ezxml"
    version = "0.8.6"
    description = "Easy-to-use C library for parsing XML documents"
    license = "MIT"
    url = "http://ezxml.sourceforge.net/"
    topics = ("xml", "parser", "c")

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "fPIC": [True, False],
        "nommap": [True, False],
    }
    default_options = {
        "fPIC": True,
        "nommap": False,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def build(self):
        cc = os.environ.get("CC", "gcc")
        ar = os.environ.get("AR", "ar")

        cflags = ["-Wall", "-O2"]
        if self.settings.build_type == "Debug":
            cflags = ["-Wall", "-O0", "-g"]

        if self.options.get_safe("fPIC"):
            cflags.append("-fPIC")
        if self.options.nommap:
            cflags.append("-D EZXML_NOMMAP")

        src = os.path.join(self.source_folder, "ezxml.c")
        obj = os.path.join(self.build_folder, "ezxml.o")
        lib = os.path.join(self.build_folder, "libezxml.a")

        self.run(f"{cc} {' '.join(cflags)} -c {src} -o {obj}")
        self.run(f"{ar} rcs {lib} {obj}")

    def package(self):
        copy(
            self,
            "ezxml.h",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "include"),
        )
        copy(
            self,
            "libezxml.a",
            src=self.build_folder,
            dst=os.path.join(self.package_folder, "lib"),
        )
        copy(
            self,
            "license.txt",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )

    def package_info(self):
        self.cpp_info.libs = ["ezxml"]
        self.cpp_info.set_property("cmake_file_name", "ezxml")
        self.cpp_info.set_property("cmake_target_name", "ezxml::ezxml")
