/* Copyright 2005-2008, Luis Furquim
 * Copyright 2015 Thiébaud Weksteen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
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

#include "conf.h"

void dump_config()
{
	unsigned int i;
	dbg("config.max_replica = %u\n", config.max_replica);
	dbg("config.max_replica_high = %u\n", config.max_replica_high);
	dbg("config.max_replica_low = %u\n", config.max_replica_low);

	dbg("config.round_robin_high = [ ");
	for(i = 0; i < config.max_replica_high; i++)
		dbg("%d ", config.round_robin_high[i]);
	dbg("]\n");

	dbg("config.round_robin_low = [ ");
	for(i = 0; i < config.max_replica_low; i++)
		dbg("%d ", config.round_robin_low[i]);
	dbg("]\n");

	dbg("config.replicas = [ \n");
	for(i = 0; i < config.max_replica; i++) {
		dbg("\t[%d] priority=%d disabled=%d path=%s pathlen=%d\n", i,
		    config.replicas[i].priority, config.replicas[i].disabled,
		    config.replicas[i].path, config.replicas[i].pathlen);
	}
	dbg("]\n");
}

/*
unsigned int get_file_max()
{
	int res;
	unsigned long long fmax;
#ifdef __linux__
	FILE *tmpf;
#else
	int    oldval;
	size_t oldlenp = sizeof(oldval);
	int    sysctl_names[] = { CTL_KERN, KERN_MAXFILESPERPROC };
#endif

#ifdef __linux__
	if((tmpf = fopen("/proc/sys/fs/nr_open", "r")) != NULL) {
		int res = fscanf(tmpf, "%lld", &fmax);
		fclose(tmpf);
	} else {
		fmax = 4096;
	}
#else
	res = sysctl(sysctl_names, 2, &oldval, &oldlenp, NULL, 0);
	if (res) {
		print_err(errno,"reading system parameter 'max open files'");
		fmax = 4096;
	} else {
		fmax = (long long unsigned int) oldval;
	}
#endif
	return fmax;
}
*/

int do_mount(char *filesystems, char *mountpoint)
{
	unsigned int i;
	char   *t, *token;
	unsigned long tmpfd;
	struct rlimit rlp;

	// Calculate config.max_replica
	// Should also get max_replica_low for exact memory allocation
	t = filesystems;
	for (i=0; t[i]; t[i]=='=' ? i++ : *t++);
	config.max_replica = i + 1;

	/*
	 * Increase process number of file descriptor up to max.
	 * Should detect if root, then use system maximum per process.
	 */
	//config.fd_buf_size = get_file_max();
	//tmpfd = (config.fd_buf_size >>= 1);
	if (getrlimit(RLIMIT_NOFILE, &rlp)) {
		print_err(errno,"reading nofile resource limit");
		exit(errno);
	}
	dbg("file limits: soft = %d hard = %d\n", rlp.rlim_cur, rlp.rlim_max);
	if (rlp.rlim_cur < rlp.rlim_max) {
		dbg("increasing soft limit to hard value\n");
		rlp.rlim_cur = rlp.rlim_max;
		if (!setrlimit(RLIMIT_NOFILE,&rlp)) {
			if (getrlimit(RLIMIT_NOFILE,&rlp)) {
				print_err(errno,"reading nofile resource limit, second attempt");
				exit(errno);
			}
			dbg("new file limits: soft = %d hard = %d\n",
			    rlp.rlim_cur, rlp.rlim_max);
		}
	}
	tmpfd = rlp.rlim_cur;
	tmpfd = (tmpfd - 6) / config.max_replica;
	dbg("max file descriptors = %u\n", tmpfd);

	/* Find closest power-of-two and create mask */
	while (config.fd_buf_size <= tmpfd) {
		config.fd_buf_size <<= 1;
	}
	config.fd_buf_size >>= 1;
	dbg("config.fd_buf_size = %#x\n",config.fd_buf_size);

	// Allocate config.tab_fd
	config.tab_fd.fd = calloc(config.fd_buf_size,sizeof(int *));
	if (!config.tab_fd.fd) {
		print_err(CHIRON3FS_ERR_LOW_MEMORY,"file descriptor hash table allocation");
		exit(CHIRON3FS_ERR_LOW_MEMORY);
	}
	for(i=0; i < config.fd_buf_size; i++) {
		config.tab_fd.fd[i] = NULL;
	}

	// Allocate config.replicas
	config.replicas = calloc(config.max_replica,sizeof(replica_t));
	if (!config.replicas) {
		print_err(errno,"replica info allocation");
		exit(errno);
	}
	for(i=0;i < config.max_replica;++i) {
		config.replicas[i].path     = NULL;
		config.replicas[i].disabled = 0;
		config.replicas[i].priority = 0;
		config.replicas[i].totrd = 0;
		config.replicas[i].totwr = 0;
	}

	// Allocate round_robin_high
	config.round_robin_high = calloc(config.max_replica,sizeof(int));
	if (!config.round_robin_high) {
		print_err(errno,"high priority round robin table allocation");
		exit(errno);
	}

	// Allocate round_robin_low
	config.round_robin_low = calloc(config.max_replica,sizeof(int));
	if (!config.round_robin_low) {
		print_err(errno,"low priority round robin table allocation");
		exit(errno);
	}

	// Split replica path into config.replicas
	for(i = 0, t = filesystems; ; t = NULL, i++)
	{
		token = strtok(t, "=");
		if(!token)
			break;
		if(token[0] == ':') {
			token += 1;
			config.replicas[i].priority = 1;
			config.round_robin_low[config.max_replica_low++] = i;
		}
		else {
			config.round_robin_high[config.max_replica_high++] = i;
		}
		config.replicas[i].path = realpath(token, NULL);
		if(!config.replicas[i].path) {
			print_err(errno, token);
			exit(errno);
		}
		config.replicas[i].pathlen = strlen(config.replicas[i].path);
		if (config.replicas[i].priority) {
			_log("Replica priority low", config.replicas[i].path, 0);
		} else {
			_log("Replica priority high", config.replicas[i].path, 0);
		}
	}

	dump_config();

	return 0;
}

/*
 * Probe each replica for filesystem feature support by performing a small
 * set of real operations inside a temporary directory.  Results are stored
 * in config.replicas[i].feature_mask and the intersection of all masks is
 * stored in config.replica_common_features.
 *
 * Returns  0 on success (or acceptable mismatch).
 * Returns -1 if a feature-set mismatch was found and the current
 *            feature_set_mismatch policy is STRICT.
 */
int probe_replica_feature_sets(void)
{
	unsigned int i;
	char probe_dir[PATH_MAX];
	char probe_file[PATH_MAX];
	char probe_sym[PATH_MAX];
	char probe_hard[PATH_MAX];
	char probe_fifo[PATH_MAX];
	struct stat st;
	int fd;

	for (i = 0; i < config.max_replica; i++) {
		if (config.replicas[i].disabled)
			continue;

		uint64_t mask = 0;

		snprintf(probe_dir,  PATH_MAX, "%s/.chiron3fs-probe", config.replicas[i].path);
		snprintf(probe_file, PATH_MAX, "%s/file",             probe_dir);
		snprintf(probe_sym,  PATH_MAX, "%s/file.sym",         probe_dir);
		snprintf(probe_hard, PATH_MAX, "%s/file.hard",        probe_dir);
		snprintf(probe_fifo, PATH_MAX, "%s/file.fifo",        probe_dir);

		/* Create the probe directory; ignore EEXIST from a previous run */
		if (mkdir(probe_dir, 0700) < 0 && errno != EEXIST) {
			fprintf(stderr,
				"chiron3fs: cannot create probe directory %s: %s\n",
				probe_dir, strerror(errno));
			return -1;
		}

		/* --- CREATE --------------------------------------------------- */
		fd = open(probe_file, O_CREAT | O_EXCL | O_WRONLY, 0600);
		if (fd >= 0) {
			close(fd);
			mask |= CHIRON_FEAT_CREATE;
		}

		if (mask & CHIRON_FEAT_CREATE) {

			/* --- SYMLINK ------------------------------------------ */
			if (symlink(probe_file, probe_sym) == 0) {
				if (lstat(probe_sym, &st) == 0 && S_ISLNK(st.st_mode))
					mask |= CHIRON_FEAT_SYMLINK;
				unlink(probe_sym);
			}

			/* --- HARDLINK ----------------------------------------- */
			if (link(probe_file, probe_hard) == 0) {
				mask |= CHIRON_FEAT_HARDLINK;
				unlink(probe_hard);
			}

			/* --- CHMOD -------------------------------------------- */
			if (chmod(probe_file, 0640) == 0) {
				if (lstat(probe_file, &st) == 0 &&
				    (st.st_mode & 0777) == 0640)
					mask |= CHIRON_FEAT_CHMOD;
				/* restore for later probes */
				chmod(probe_file, 0600);
			}

			/* --- UTIMENS ------------------------------------------ */
			{
				struct timespec tv[2];
				tv[0].tv_sec  = 1000000000;
				tv[0].tv_nsec = 0;
				tv[1].tv_sec  = 1000000000;
				tv[1].tv_nsec = 0;
				if (utimensat(AT_FDCWD, probe_file, tv, 0) == 0)
					mask |= CHIRON_FEAT_UTIMENS;
			}

			/* --- FIFO --------------------------------------------- */
			if (mkfifo(probe_fifo, 0600) == 0) {
				mask |= CHIRON_FEAT_FIFO;
				unlink(probe_fifo);
			}

			unlink(probe_file);
		}

		rmdir(probe_dir);

		config.replicas[i].feature_mask = mask;
		dbg("replica[%u] %s feature_mask=0x%llx\n",
		    i, config.replicas[i].path,
		    (unsigned long long)mask);
	}

	/* Compute common (intersection) feature mask across all replicas */
	uint64_t common = ~0ULL;
	for (i = 0; i < config.max_replica; i++) {
		if (!config.replicas[i].disabled)
			common &= config.replicas[i].feature_mask;
	}
	config.replica_common_features = common;

	/* Detect mismatches */
	int mismatch = 0;
	uint64_t ref = 0;
	int ref_set = 0;
	for (i = 0; i < config.max_replica; i++) {
		if (config.replicas[i].disabled)
			continue;
		if (!ref_set) {
			ref = config.replicas[i].feature_mask;
			ref_set = 1;
		} else if (config.replicas[i].feature_mask != ref) {
			mismatch = 1;
		}
	}

	if (!mismatch)
		return 0;

	/* Report the mismatch with per-replica detail */
	fprintf(stderr, "chiron3fs: feature-set mismatch detected between replicas:\n");
	for (i = 0; i < config.max_replica; i++) {
		if (config.replicas[i].disabled)
			continue;
		uint64_t m = config.replicas[i].feature_mask;
		fprintf(stderr,
			"  replica[%u] %s: %s%s%s%s%s%s\n",
			i, config.replicas[i].path,
			(m & CHIRON_FEAT_CREATE)   ? "create "   : "",
			(m & CHIRON_FEAT_SYMLINK)  ? "symlink "  : "",
			(m & CHIRON_FEAT_HARDLINK) ? "hardlink " : "",
			(m & CHIRON_FEAT_CHMOD)    ? "chmod "    : "",
			(m & CHIRON_FEAT_UTIMENS)  ? "utimens "  : "",
			(m & CHIRON_FEAT_FIFO)     ? "fifo"      : "");
	}

	switch (config.feature_set_mismatch) {
	case CHIRON_FEAT_MISMATCH_STRICT:
		fprintf(stderr,
			"chiron3fs: refusing to mount due to feature-set mismatch.\n"
			"  Use --feature-set-mismatch=ignore      to mount without restrictions.\n"
			"  Use --feature-set-mismatch=compatibility to disable unsupported operations.\n");
		return -1;

	case CHIRON_FEAT_MISMATCH_COMPAT:
		fprintf(stderr,
			"chiron3fs: compatibility mode: operations unsupported by any replica "
			"will be disabled (ENOSYS).\n");
		break;

	case CHIRON_FEAT_MISMATCH_IGNORE:
		fprintf(stderr,
			"chiron3fs: ignoring feature-set mismatch (replicas may be disabled at runtime).\n");
		break;
	}

	return 0;
}
