/* Copyright 2005-2008 Luis Furquim
 * Copyright 2015 Thiébaud Weksteen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef CHIRON3FS_FS_H
#define CHIRON3FS_FS_H


#include "common.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <libgen.h>
#include <pthread.h>
#include <getopt.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/un.h>
#include <sys/socket.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif


#ifdef HAVE_GETMNTENT
#include <mntent.h>
#else
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif

#include "chiron-types.h"
#include "conf.h"
#include "debug.h"
#include "utils.h"
#include "hash.h"

/*
 * Feature bits used for startup probing of replica capabilities.
 */
#define CHIRON_FEAT_CREATE   (1ULL << 0)  /* create/unlink regular file */
#define CHIRON_FEAT_SYMLINK  (1ULL << 1)  /* symlink + readlink */
#define CHIRON_FEAT_HARDLINK (1ULL << 2)  /* hard link (link()) */
#define CHIRON_FEAT_CHMOD    (1ULL << 3)  /* chmod with mode verification */
#define CHIRON_FEAT_UTIMENS  (1ULL << 4)  /* utimensat */
#define CHIRON_FEAT_FIFO     (1ULL << 5)  /* mkfifo */

/*
 * Values for feature_set_mismatch in chiron3fs_config.
 *
 * STRICT       (0) — default: refuse to mount when replicas differ.
 * IGNORE       (1) — mount anyway; diverging replicas will be disabled
 *                    at runtime as today.
 * COMPAT       (2) — mount, but permanently disable FUSE operations that
 *                    are not supported by every replica, so that
 *                    unsupported features return ENOSYS rather than
 *                    silently disabling a replica later.
 */
#define CHIRON_FEAT_MISMATCH_STRICT 0
#define CHIRON_FEAT_MISMATCH_IGNORE 1
#define CHIRON_FEAT_MISMATCH_COMPAT 2

/*
 * Main configuration structure for ChironFS.
 */
struct chiron3fs_config {
	unsigned int max_replica;
	unsigned int max_replica_high;
	unsigned int max_replica_low;
	unsigned int curr_replica_high;
	unsigned int curr_replica_low;
	unsigned int *round_robin_high;
	unsigned int *round_robin_low;
	replica_t     *replicas;
	char         *mountpoint;
	char         *ctl_socket_name;
	fd_t         tab_fd;
	uid_t	     uid;
	gid_t        gid;
	uint64_t     fd_buf_size;
	int          feature_set_mismatch;     /* one of CHIRON_FEAT_MISMATCH_* */
	uint64_t     replica_common_features;  /* intersection of all replica masks */
};

extern struct chiron3fs_config config;

/*
 * This structure is used to parse command line options using the
 * fuse_opt_parse wrapper. Its content is not used once initialisation
 * is finished.
 */
struct chiron3fs_options {
	char *ctl_socket_name;
	char *replica_args;
	char *logname;
	char *mountpoint;
	char *feature_set_mismatch;
	int  quiet;
};

extern struct chiron3fs_options options;
#define CHIRON_OPT(t, p, v) { t, offsetof(struct chiron3fs_options, p), v }
enum {
	KEY_HELP,
	KEY_VERSION
};

/*
 * Actions exposed via control socket
 */
enum {
	GET_MAX_REPLICA,
	GET_REPLICA_STATUS,
	GET_REPLICA_PATH,
	GET_REPLICA_TOTAL_READ,
	GET_REPLICA_TOTAL_WRITE,
	GET_REPLICA_PRIORITY,
	DONE
};

void help(void);
unsigned hash_fd(unsigned fd_main);
int fd_hashseekfree(unsigned fd_ndx);
int fd_hashset(int *fd);
char *xlate(const char *fname, char *rpath);
int choose_replica(int try);
void disable_replica(int n);
void enable_replica(int n);
void print_version(void);
void *ctl_server(void *arg);

#endif /* CHIRON3FS_FS_H */
