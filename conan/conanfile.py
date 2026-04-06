from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, CMake
from conan.tools.system.package_manager import Dnf, Yum

class SteroidsConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    default_options = {
        "sdl/*:shared": False,
        "sdl/*:video": True,
        "sdl/*:wayland": True,
        "sdl/*:x11": True,
        "sdl/*:opengl": True,
        "sdl/*:gpu": True,
        "sdl/*:render": True,
        "sdl/*:opengles": True,
        "sdl/*:xcursor": True,
        "sdl/*:xdbe": True,
        "sdl/*:xfixes": True,
        "sdl/*:xinput": True,
        "sdl/*:xrandr": True,
        "sdl/*:xscrnsaver": True,
        "sdl/*:xshape": True,
        "sdl/*:xsync": True,
        "sdl/*:audio": False,
        "sdl/*:alsa": False,
        "sdl/*:pulseaudio": False,
        "sdl/*:sndio": False,
        "sdl/*:camera": False,
        "sdl/*:haptic": False,
        "sdl/*:joystick": False,
        "sdl/*:sensor": False,
        "sdl/*:tray": False,
        "sdl/*:libudev": False,
        "sdl/*:dialog": False,
        "sdl/*:hidapi": False,
        "sdl/*:power": False,
        "sdl/*:vulkan": False,
        "sdl/*:dbus": False,
    }
    
    def system_requirements(self):
        # Install system libraries for X11 and OpenGL/EGL
        dnf = Dnf(self)
        dnf.install([
            "pkgconf",
            "libX11-devel",
            "libXrandr-devel",
            "libglvnd-devel",
            "mesa-libEGL-devel",
        ])
        
        yum = Yum(self)
        yum.install([
            "pkgconfig",
            "libX11-devel",
            "libXrandr-devel",
            "libglvnd-devel",
            "mesa-libEGL-devel",
        ])
    
    def requirements(self):
        # Custom recipes (local)
        self.requires("cfgpath/cci.20260404")
        self.requires("hash_table/cci.20260404")
        self.requires("ketopt/cci.20260404")
        self.requires("libsir/2.2.5")
        self.requires("lwrb/3.2.0")
        self.requires("scv/cci.20260404")
        
        # Dependencies from ConanCenter
        self.requires("cwalk/1.2.8")
        self.requires("inih/62")
        self.requires("kuba-zip/0.3.2")
        self.requires("libpng/1.6.56")
        self.requires("luajit/2.1.0-beta3")
        self.requires("sdl/3.4.0")
        self.requires("stb/cci.20240531")
    
    def build_requirements(self):
        # Build tools and CMake modules
        self.tool_requires("cmake_barebones/cci.20260404")
    
    def generate(self):
        # Warmup mode for Docker image build: populate Conan cache only.
        if self.conf.get("user.steroids:cache_warmup", default=False, 
         check_type=bool):
            self.output.info("Cache warmup mode enabled: skipping generators")
            return

        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()
