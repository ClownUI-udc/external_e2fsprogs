/*
 * llseek.c -- stub calling the llseek system call
 *
 * Copyright (C) 1994, 1995, 1996, 1997 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <errno.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef __MSDOS__
#include <io.h>
#endif
#include "et/com_err.h"
#include "ext2fs/ext2_io.h"

#ifdef __linux__

#ifdef HAVE_LLSEEK
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <syscall.h>

#if (__GLIBC__ == 2)
ext2_loff_t llseek (int fd, ext2_loff_t offset, int origin);
#endif

#define my_llseek llseek

#else	/* HAVE_LLSEEK */

#ifdef __alpha__

#define llseek lseek

#else /* !__alpha__ */

#include <linux/unistd.h>

#ifndef __NR__llseek
#define __NR__llseek            140
#endif

#ifndef __i386__
static int _llseek (unsigned int, unsigned long,
		   unsigned long, ext2_loff_t *, unsigned int);

static _syscall5(int,_llseek,unsigned int,fd,unsigned long,offset_high,
		 unsigned long, offset_low,ext2_loff_t *,result,
		 unsigned int, origin)
#endif

static ext2_loff_t my_llseek (int fd, ext2_loff_t offset, int origin)
{
	ext2_loff_t result;
	int retval;

#ifndef __i386__
	retval = _llseek(fd, ((unsigned long long) offset) >> 32,
#else			  
	retval = syscall(__NR__llseek, fd, (unsigned long long) (offset >> 32),
#endif
			  ((unsigned long long) offset) & 0xffffffff,
			&result, origin);
	return (retval == -1 ? (ext2_loff_t) retval : result);
}

#endif	/* HAVE_LLSEEK */

#endif /* __alpha__ */

ext2_loff_t ext2fs_llseek (int fd, ext2_loff_t offset, int origin)
{
	ext2_loff_t result;
	static int do_compat = 0;

	if ((sizeof(off_t) >= sizeof(ext2_loff_t)) ||
	    (offset < ((ext2_loff_t) 1 << ((sizeof(off_t)*8) -1))))
		return lseek(fd, (off_t) offset, origin);

	if (do_compat) {
		errno = EINVAL;
		return -1;
	}
	
	result = my_llseek (fd, offset, origin);
	if (result == -1 && errno == ENOSYS) {
		/*
		 * Just in case this code runs on top of an old kernel
		 * which does not support the llseek system call
		 */
		do_compat++;
		errno = EINVAL;
	}
	return result;
}

#else /* !linux */

ext2_loff_t ext2fs_llseek (int fd, ext2_loff_t offset, int origin)
{
	if ((sizeof(off_t) < sizeof(ext2_loff_t)) &&
	    (offset >= ((ext2_loff_t) 1 << ((sizeof(off_t)*8) -1)))) {
		errno = EINVAL;
		return -1;
	}
	return lseek (fd, (off_t) offset, origin);
}

#endif 	/* linux */


