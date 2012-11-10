top_builddir = ./

.PHONY: all clean

all: ipcmgr

clean:
		${RM} ./build/$@

ipcmgr: ipcmgr.c
		${CC} -ggdb -o ./build/$@ $?
