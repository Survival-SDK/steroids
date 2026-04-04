all: debug

include barebones.mk

TRIPLET=any-any-any

ifeq ($(TRIPLET),any-any-any)
    TOOLCHAIN_OPT=toolchain=cc
    INCLUDE_OPT=--include-dir=/usr/local/include
else
    TOOLCHAIN_OPT=toolchain=$(TRIPLET)-gcc
    INCLUDE_OPT=--include-dir=/usr/local/$(TRIPLET)/include
endif

run-image:
	docker run --net=host -i -t -v ~/.vgazer:/root/.vgazer \
     -v `pwd`:/mnt/steroids --entrypoint sh steroids-deps-$(TRIPLET)

release:
	docker run --net=host -i -t \
     -v ~/.vgazer:/root/.vgazer -v `pwd`:/mnt/steroids \
     --entrypoint sh steroids-deps-$(TRIPLET) \
     -E -c "make bb_build_release $(TOOLCHAIN_OPT) $(INCLUDE_OPT)" \
    | tee build.log

debug:
	docker run --net=host -i -t \
     -v ~/.vgazer:/root/.vgazer -v `pwd`:/mnt/steroids \
     --entrypoint sh steroids-deps-$(TRIPLET) \
     -E -c "make bb_build_debug $(TOOLCHAIN_OPT) $(INCLUDE_OPT)" | tee build.log

coverage:
	docker run --net=host -i -t \
     -v ~/.vgazer:/root/.vgazer -v `pwd`:/mnt/steroids \
     --entrypoint sh steroids-deps-$(TRIPLET) \
     -E -c "make bb_build_coverage $(TOOLCHAIN_OPT) $(INCLUDE_OPT)" \
    | tee build.log

lint:
	docker run --net=host -i -t \
     -v ~/.vgazer:/root/.vgazer -v `pwd`:/mnt/steroids \
     --entrypoint sh steroids-deps-$(TRIPLET) \
     -E -c "make bb_build_lint $(TOOLCHAIN_OPT) $(INCLUDE_OPT)" | tee build.log

iwyu:
	docker run --net=host -i -t \
     -v ~/.vgazer:/root/.vgazer -v `pwd`:/mnt/steroids \
     --entrypoint sh steroids-deps-$(TRIPLET) \
     -E -c "make bb_build_iwyu $(TOOLCHAIN_OPT) $(INCLUDE_OPT)" | tee build.log

lint_build: bb_lint_build

install: bb_install

clean: bb_clean

dockerfiles/deps-$(TRIPLET).dockerfile: deps/$(TRIPLET) scripts/build_dockerfile.py
	scripts/build_dockerfile.py --target=$(TRIPLET)

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
