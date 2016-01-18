PROGRAMS=
SUBDIRS=src scripts
LIBRARIES=
ARCHIVES=
HEADERS:=$(wildcard inc/*.h)

include ${MAKE_INC}/workspace.mak
