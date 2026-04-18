AM_CFLAGS = -Wall

bin_PROGRAMS = chiron3fs chiron3ctl
chiron3fs_SOURCES = fs.c conf.c debug.c hash.c
chiron3fs_CFLAGS = $(CFLAGS) $(FUSE_CFLAGS)
chiron3fs_LDADD = $(FUSE_LIBS)
chiron3ctl_SOURCES = ctl.c
chiron3ctl_CFLAGS = $(CFLAGS) $(FUSE_CFLAGS)
