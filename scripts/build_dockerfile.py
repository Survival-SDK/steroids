#!/usr/bin/env python3

import os
from pathlib import Path
import argparse

def GetDepsfileName(target):
    return "deps/{target}".format(target=target)

def GetDockefrileName(target):
    return "dockerfiles/deps-{target}.dockerfile".format(target=target)

def GetBaseImageName(target):
    return {
        "any-any-any": "fedora:40",
        "x86_64-linux-gnu": "amazonlinux:2",
        "x86_64-steamrt-scout":
            "registry.gitlab.steamos.cloud/steamrt/scout/sdk:"
            "latest-steam-client-general-availability",
        "x86_64-steamrt-sniper":
            "registry.gitlab.steamos.cloud/steamrt/sniper/sdk:"
            "latest-steam-client-general-availability",
        "x86_64-w64-mingw32": "archlinux:latest",
    }[target]

def TargetIsHost(target):
    return {
        "any-any-any":           True,
        "x86_64-linux-gnu":      False,
        "x86_64-steamrt-scout":  True,
        "x86_64-steamrt-sniper": True,
        "x86_64-w64-mingw32":    False,
    }[target]

def GetPipInstall(target):
    return {
        "any-any-any":           "pip3 install --break-system-packages",
        "x86_64-linux-gnu":      "pip3 install",
        "x86_64-steamrt-scout":  "pip3 install",
        "x86_64-steamrt-sniper": "pip3 install",
        "x86_64-w64-mingw32":    "pip3 install --break-system-packages",
    }[target]

def GetPreInstallCmds(target):
    return {
        "any-any-any":           "groupadd --gid $GROUP_ID user \\\n"
                              "&& useradd -m --uid $USER_ID --gid $GROUP_ID user \\\n"
                              "&& dnf -y install python3 python3-pip git \\\n",
        "x86_64-linux-gnu":      "yum -y install shadow-utils\\\n"
                              "&& groupadd --gid $GROUP_ID user \\\n"
                              "&& useradd -m --uid $USER_ID --gid $GROUP_ID user \\\n"
                              "&& yum -y install python3 python3-pip git \\\n"
                              "&& {pip_install} urllib3==1.26.6".format(
                                pip_install=GetPipInstall(target)
                              ),
        "x86_64-steamrt-scout":  "TODO",
        "x86_64-steamrt-sniper": "TODO",
        "x86_64-w64-mingw32":    "groupadd --gid $GROUP_ID user \\\n"
                              "&& useradd -m --uid $USER_ID --gid $GROUP_ID user \\\n"
                              "&& pacman-key --init \\\n"
                              "&& pacman --noconfirm --disable-download-timeout -Syu \\\n"
                              "&& pacman --noconfirm --disable-download-timeout -S python python-pip git \\\n",
    }[target]

def GenerateDockerfile(target):
    depsFile = GetDepsfileName(target)
    dockerfileName = GetDockefrileName(target)
    baseImageName = GetBaseImageName(target)
    preInstallCmds = GetPreInstallCmds(target)
    pipInstall = GetPipInstall(target)

    isHost = TargetIsHost(target)

    targetArg = "" if TargetIsHost(target) else "--target={target}".format(
     target=target)

    with open(dockerfileName, "w") as dockerfile:
        dockerfile.write(
            "FROM {baseImageName} as build \n"
            "MAINTAINER Vasiliy Edomin <Vasiliy.Edomin@gmail.com> \n"
            "ARG USER_ID \n"
            "ARG GROUP_ID \n"
            "RUN {preInstallCmds} \\\n"
            "&& {pipInstall} vgazer \n"
            "WORKDIR /mnt/steroids \n"
            "RUN --mount=type=bind,source=.,target=/mnt/steroids vgazer install {targetArg} {depsFile} \n"
            "USER user \n"
            "".format(baseImageName=baseImageName,
             preInstallCmds=preInstallCmds, pipInstall=pipInstall,
             targetArg=targetArg, depsFile=depsFile)
        )

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--target")
    args = parser.parse_args()
    if not os.path.exists("./dockerfiles"):
        os.makedirs("./dockerfiles")
    GenerateDockerfile(args.target)

if __name__ == "__main__":
    main()
