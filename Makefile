# The llvm-mos SDK and c1541 run from pinned images; set MOS_CC or C1541 to use
# host installs instead. HOME must be writable by the calling uid or VICE logs
# errors creating its config, cache and state directories.
MOS_IMAGE ?= ghcr.io/anarkiwi/docker-mos-llvm-sdk:v23.0.1
VICE_IMAGE ?= anarkiwi/asid-vice:3.10.0.0
DOCKER_RUN := docker run --rm -u $(shell id -u):$(shell id -g) \
    -v $(CURDIR):/work -w /work
MOS_CC ?= $(DOCKER_RUN) $(MOS_IMAGE) mos-c64-clang
C1541 ?= $(DOCKER_RUN) -e HOME=/tmp --entrypoint c1541 $(VICE_IMAGE)

CFLAGS := -Wall -Os -fnonreentrant

all: vvf.d64 vvf.prg

vvf.prg: vvf.c vessel.h midi.h
	$(MOS_CC) $(CFLAGS) -o $@ $<

vvf.d64: vvf.prg
	$(C1541) -format diskname,id d64 vvf.d64 -attach vvf.d64 -write vvf.prg vvf

clean:
	rm -f vvf.prg vvf.d64 *.elf

upload: all
	ncftpput -p "" -Cv c64 vvf.prg /Temp/vvf.prg
