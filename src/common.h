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

#ifndef CHIRON3FS_COMMON_H
#define CHIRON3FS_COMMON_H
#include "config.h"

#ifdef __linux__
#define _GNU_SOURCE
#endif

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <fuse_opt.h>

#define CHIRON3FS_ERR_LOW_MEMORY        -1
#define CHIRON3FS_ERR_LOG_ON_MOUNTPOINT -2
#define CHIRON3FS_ERR_BAD_OPTIONS       -3
#define CHIRON3FS_ERR_TOO_MANY_FOPENS   -4
#define CHIRON3FS_ERR_BAD_LOG_FILE      -5
#define CHIRON3FS_INVALID_PATH_MAX      -6
#define CHIRON3FS_ADM_FORCED            -7

#endif /* CHIRON3FS_COMMON_H */
