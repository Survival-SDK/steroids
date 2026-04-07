all: debug

include barebones.mk

TRIPLET=any-any-any

run-image:
	docker run --net=host -i -t -v ~/.vgazer:/root/.vgazer \
     -v `pwd`:/mnt/steroids --entrypoint sh steroids-deps-$(TRIPLET)

relwithdebinfo:
	docker run --net=host -i -t \
     -v `pwd`:/mnt/steroids \
     --entrypoint sh steroids-deps-$(TRIPLET) \
     -E -c "cd /mnt/steroids && \
            mkdir -p cmake_build && \
            conan install ./conan \
             --profile:host=conan/profiles/$(TRIPLET)-$@.profile \
             --profile:build=conan/profiles/$(TRIPLET)-build.profile \
             --output-folder=cmake_build --build=missing && \
            cmake -B cmake_build -DCMAKE_BUILD_TYPE=RelWithDebInfo \
             -DCMAKE_TOOLCHAIN_FILE=cmake_build/conan_toolchain.cmake  \
             -DBB_MORE_WARNINGS=ON -DBB_WERROR=ON && \
            cmake --build cmake_build" | tee build.log

debug:
	docker run --net=host -i -t \
     -v `pwd`:/mnt/steroids \
     --entrypoint sh steroids-deps-$(TRIPLET) \
     -E -c "cd /mnt/steroids && \
            mkdir -p cmake_build && \
            conan install ./conan \
             --profile:host=conan/profiles/$(TRIPLET)-$@.profile \
             --profile:build=conan/profiles/$(TRIPLET)-build.profile \
             --output-folder=cmake_build --build=missing && \
            cmake -B cmake_build -DCMAKE_BUILD_TYPE=Debug \
             -DCMAKE_TOOLCHAIN_FILE=cmake_build/conan_toolchain.cmake \
             -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
             -DBB_MORE_WARNINGS=ON -DBB_WERROR=ON && \
            cmake --build cmake_build" | tee build.log

# coverage:
# 	docker run --net=host -i -t \
#      -v `pwd`:/mnt/steroids \
#      --entrypoint sh steroids-deps-$(TRIPLET) \
#      -E -c "cd /mnt/steroids && \
#             mkdir -p cmake_build && \
#             conan install ./conan \
#              --profile:host=conan/profiles/$(TRIPLET)-$@.profile \
#              --profile:build=conan/profiles/$(TRIPLET)-build.profile \
#              --output-folder=cmake_build --build=missing && \
#             cmake -B cmake_build -DCMAKE_BUILD_TYPE=Debug \
#              -DCMAKE_TOOLCHAIN_FILE=cmake_build/conan_toolchain.cmake \
#              -DCMAKE_C_FLAGS='--coverage' -DCMAKE_CXX_FLAGS='--coverage' 
          #    -DBB_MORE_WARNINGS=ON -DBB_WERROR=ON && \
#             cmake --build cmake_build" | tee build.log

# lint:
# 	docker run --net=host -i -t \
#      -v `pwd`:/mnt/steroids \
#      --entrypoint sh steroids-deps-$(TRIPLET) \
#      -E -c "cd /mnt/steroids && \
#             mkdir -p cmake_build && \
#             conan install ./conan \
#              --profile:host=conan/profiles/$(TRIPLET)-$@.profile \
#              --profile:build=conan/profiles/$(TRIPLET)-build.profile \
#              --output-folder=cmake_build --build=missing && \
#             cmake -B cmake_build -DCMAKE_BUILD_TYPE=Debug \
#              -DCMAKE_TOOLCHAIN_FILE=cmake_build/conan_toolchain.cmake \
#              -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
#              -DCMAKE_C_CLANG_TIDY='clang-tidy' 
          #    -DBB_MORE_WARNINGS=ON -DBB_WERROR=ON && \
#             cmake --build cmake_build" | tee build.log

# iwyu:
# 	docker run --net=host -i -t \
#      -v `pwd`:/mnt/steroids \
#      --entrypoint sh steroids-deps-$(TRIPLET) \
#      -E -c "cd /mnt/steroids && \
#             mkdir -p cmake_build && \
#             conan install ./conan \
#              --profile:host=conan/profiles/$(TRIPLET)-$@.profile \
#              --profile:build=conan/profiles/$(TRIPLET)-build.profile \
#              --output-folder=cmake_build --build=missing && \
#             cmake -B cmake_build -DCMAKE_BUILD_TYPE=Debug \
#              -DCMAKE_TOOLCHAIN_FILE=cmake_build/conan_toolchain.cmake \
#              -DCMAKE_C_INCLUDE_WHAT_YOU_USE='iwyu' \
#              -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE='iwyu' 
          #    -DBB_MORE_WARNINGS=ON -DBB_WERROR=ON && \
#             cmake --build cmake_build" | tee build.log

lint_build: bb_lint_build

install: bb_install

clean: bb_clean

build-image: dockerfiles/deps-$(TRIPLET).dockerfile
	docker build --progress=plain --network=host --no-cache \
     -f dockerfiles/deps-$(TRIPLET).dockerfile -t steroids-deps-$(TRIPLET) \
     --build-arg USER_ID=$(SUDO_UID) --build-arg GROUP_ID=$(SUDO_GID) .

build-image-with-cache: dockerfiles/deps-$(TRIPLET).dockerfile
	docker build --progress=plain --network=host \
     -f dockerfiles/deps-$(TRIPLET).dockerfile -t steroids-deps-$(TRIPLET) \
     --build-arg USER_ID=$(SUDO_UID) --build-arg GROUP_ID=$(SUDO_GID) .

build-container:
	-distrobox-stop --root -Y steroids-deps-$(TRIPLET)
	-distrobox-rm --root -Y steroids-deps-$(TRIPLET)
	distrobox-create --root --image steroids-deps-$(TRIPLET) \
     --name steroids-deps-$(TRIPLET)

run:
	distrobox enter --root steroids-deps-$(TRIPLET) -- ./cmake_build/steroids \
      --cfg=./cmake_build/steroids.ini

run-verbose-dev:
	distrobox enter --root steroids-deps-$(TRIPLET) --verbose -- \
      ./cmake_build/steroids --cfg=./cmake_build/steroids.ini

run-help:
	distrobox enter --root steroids-deps-$(TRIPLET) -- ./cmake_build/steroids \
     --cfg=./cmake_build/steroids.ini --help

run-debug-shell:
	distrobox enter --root steroids-deps-$(TRIPLET)

valgrind:
	distrobox enter --root \
     steroids-deps-$(TRIPLET) -- valgrind --tool=memcheck ./cmake_build/steroids \
     --cfg=./cmake_build/steroids.ini
