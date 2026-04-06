[settings]
os=Linux
arch=x86_64
compiler=gcc
compiler.version=7
compiler.libcxx=libstdc++11
build_type=RelWithDebInfo

[options]
sdl/*:x11=False
xkbcommon/*:with_x11=False

[conf]
tools.system.package_manager:mode=check
tools.system.package_manager:sudo=False
