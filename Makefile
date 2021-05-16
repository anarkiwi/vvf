all: vvf.d64 vvf.prg

vvf.prg: vvf.c vessel.h
	cl65 -Osir -Cl vvf.c -o vvf.prg

vvf.d64: vvf.prg
	c1541 -format diskname,id d64 vvf.d64 -attach vvf.d64 -write vvf.prg vvf

upload: all
	ncftpput -p "" -Cv c64 vvf.prg /Temp/vvf.prg
