SUBDIRS=yang-model src scripts
HEADERS:=$(wildcard inc/*.h)

include ${MAKE_INC}/workspace.mak
