# c1541 runs from a pinned image; set C1541 to use a host VICE install instead.
# HOME must be writable by the calling uid or VICE logs errors creating its
# config, cache and state directories.
VICE_IMAGE ?= anarkiwi/asid-vice:3.10.0.0
DOCKER_RUN := docker run --rm -u $(shell id -u):$(shell id -g) \
    -v $(CURDIR):/work -w /work
C1541 ?= $(DOCKER_RUN) -e HOME=/tmp --entrypoint c1541 $(VICE_IMAGE)

all: vvf.d64 vvf.prg

vvf.prg: vvf.c vessel.h
	cl65 -Osir -Cl vvf.c -o vvf.prg

vvf.d64: vvf.prg
	$(C1541) -format diskname,id d64 vvf.d64 -attach vvf.d64 -write vvf.prg vvf

upload: all
	ncftpput -p "" -Cv c64 vvf.prg /Temp/vvf.prg
