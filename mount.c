/*
 * getsize.c --- get the size of a partition.
 * 
 * Copyright (C) 1995 Theodore Ts'o.  This file may be
 * redistributed under the terms of the GNU Public License.
 */



#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <linux/fd.h>
#include <mntent.h>
/*
#ifdef HAVE_GETMNTINFO
#include <paths.h>
#include <sys/param.h>
#include <sys/mount.h>
#endif 
*/

#include <linux/ext2_fs.h>

#define HAVE_MNTENT_H 1
/*
 * ext2fs_check_if_mounted flags
 */
#define EXT2_MF_MOUNTED		1
#define EXT2_MF_ISROOT		2
#define EXT2_MF_READONLY	4 


#ifdef HAVE_MNTENT_H
/*
 * XXX we only check to see if the mount is readonly when it's the
 * root filesystem.
 */
int check_mntent(const char *file, int *mount_flags)
{
	FILE * f;
	struct mntent * mnt;
	int	fd;

	*mount_flags = 0;
	if ((f = setmntent (MOUNTED, "r")) == NULL)
		return errno;
	while ((mnt = getmntent (f)) != NULL)
		if (strcmp(file, mnt->mnt_fsname) == 0)
			break;
	endmntent (f);
	if (mnt == 0)
		return 0;
	*mount_flags = EXT2_MF_MOUNTED;
	
	if (!strcmp(mnt->mnt_dir, "/")) {
		*mount_flags |= EXT2_MF_ISROOT;
		fd = open(MOUNTED, O_RDWR);
		if (fd < 0) {
			if (errno == EROFS)
				*mount_flags |= EXT2_MF_READONLY;
		} else
			close(fd);
	}
	return 0;
}
#endif

#ifdef HAVE_GETMNTINFO
int  check_getmntinfo(const char *file, int *mount_flags)
{
	struct statfs *mp;
        int    len, n;
        const  char   *s1;
	char	*s2;

        n = getmntinfo(&mp, MNT_NOWAIT);
        if (n == 0)
		return errno;

        len = sizeof(_PATH_DEV) - 1;
        s1 = file;
        if (strncmp(_PATH_DEV, s1, len) == 0)
                s1 += len;
 
	*mount_flags = 0;
        while (--n >= 0) {
                s2 = mp->f_mntfromname;
                if (strncmp(_PATH_DEV, s2, len) == 0) {
                        s2 += len - 1;
                        *s2 = 'r';
                }
                if (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = EXT2_MF_MOUNTED;
			break;
		}
                ++mp;
	}
	return 0;
}
#endif /* HAVE_GETMNTINFO */

/*
 * Is_mounted is set to 1 if the device is mounted, 0 otherwise
 */
int  check_if_mounted(const char *file, int *mount_flags)
{
#ifdef HAVE_MNTENT_H
	return check_mntent(file, mount_flags);
#else 
#ifdef HAVE_GETMNTINFO
	return check_getmntinfo(file, mount_flags);
#else
	*mount_flags = 0;
	return 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
}
