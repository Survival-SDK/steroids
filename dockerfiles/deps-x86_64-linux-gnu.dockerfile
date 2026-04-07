FROM amazonlinux:2 AS build
LABEL org.opencontainers.image.authors="Vasiliy Edomin <vasiliy.edomin@gmail.com>"
ARG USER_ID
ARG GROUP_ID

RUN yum -y update \
 && yum -y install shadow-utils \
 && groupadd --gid $GROUP_ID user \
 && useradd -m --uid $USER_ID --gid $GROUP_ID user \
 && yum -y --setopt=install_weak_deps=False install \
    python3 \
    python3-pip \
    git \
    gcc \
    gcc-c++ \
    make \
    autoconf \
    automake \
    bison \
    cmake3 \
    flex \
    libtool \
    m4 \
    meson \
    ninja-build \
    pkgconf \
    yasm \
    perl-Digest-SHA \
    perl-FindBin \
    perl-IPC-Cmd \
    perl-Time-Piece \
    libX11-devel \
    libXrandr-devel \
    libglvnd-devel \
    mesa-libEGL-devel \
    clang-tools-extra \
    iwyu \
    valgrind \
 && ln -s /usr/bin/cmake3 /usr/bin/cmake

ARG CONAN_VERSION=2.27.0
RUN pip3 install "urllib3<2" "conan==${CONAN_VERSION}"

ENV CONAN_HOME=/var/conan2

RUN mkdir -p "${CONAN_HOME}"

RUN --mount=type=bind,source=.,target=/tmp/steroids \
    conan export /tmp/steroids/conan/recipes/cfgpath/all \
 && conan export /tmp/steroids/conan/recipes/cmake_barebones/all \
 && conan export /tmp/steroids/conan/recipes/ezxml/all \
 && conan export /tmp/steroids/conan/recipes/hash_table/all \
 && conan export /tmp/steroids/conan/recipes/ketopt/all \
 && conan export /tmp/steroids/conan/recipes/libsir/all \
 && conan export /tmp/steroids/conan/recipes/lwrb/all \
 && conan export /tmp/steroids/conan/recipes/scv/all

RUN --mount=type=bind,source=.,target=/tmp/steroids \
    conan install /tmp/steroids/conan \
    --profile:host=/tmp/steroids/conan/profiles/x86_64-linux-gnu-relwithdebinfo.profile \
    --profile:build=/tmp/steroids/conan/profiles/x86_64-linux-gnu-build.profile \
    --build=missing --build=m4/* \
    -c:h tools.system.package_manager:mode=install \
    -c:h tools.system.package_manager:sudo=False \
    -c:h user.steroids:cache_warmup=True \
    --output-folder=/tmp/conan-install

RUN chown -R user:user "${CONAN_HOME}"

USER user

WORKDIR /mnt/steroids
