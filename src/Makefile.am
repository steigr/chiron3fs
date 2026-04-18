AM_CFLAGS = -Wall

noinst_HEADERS = chiron-types.h common.h conf.h ctl.h debug.h fs.h hash.h utils.h

bin_PROGRAMS = chiron3fs chiron3ctl
chiron3fs_SOURCES = fs.c conf.c debug.c hash.c
chiron3fs_CFLAGS = $(CFLAGS) $(FUSE_CFLAGS)
chiron3fs_LDADD = $(FUSE_LIBS)
chiron3ctl_SOURCES = ctl.c
chiron3ctl_CFLAGS = $(CFLAGS) $(FUSE_CFLAGS)
