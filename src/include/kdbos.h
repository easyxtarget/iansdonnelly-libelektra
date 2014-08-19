/***************************************************************************
             kdbos.h  -  operating system specific workarounds
                             -------------------
    begin                : Mon Dec 29 2003
    copyright            : (C) 2003 by Avi Alkalay
    email                : avi@unix.sh
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

/* This header purpose is that afterwards following types are defined:
 * .. means don't care, just enough for your system
 * For more information on that types read POSIX documentation.
 *
 *
 * Integer Types must be at least 32bit:
 *
 * Type     Purpose                 Limits
 * int      Integral Fast Type      INT_MIN, INT_MAX
 * size_t   size of array or string 0, SIZE_MAX
 * ssize_t  size with error cond.   -1, SSIZE_MAX(<SIZE_MAX)
 * time_t   Seconds since 1970      0,.. recommended: 64 bit
 * uid_t    User identification     0,..
 * gid_t    Group identification    0,..
 *
 *
 * Following Elektra specific types must be defined with at least 32 bit:
 *
 * Type     Purpose
 * keyswitch_t For keyNew
 * option_t    For kdbGet, kdbSet and ksLookup*
 * cursor_t stores information to find a position in a keyset
 *
 * Following constants must be defined:
 *
 * KDB_PATH_SEPARATOR  how to delimit pathnames
 * KDB_FILE_MODE       the standard mode for keys
 * KDB_DIR_MODE        the mode to add (|=) for key directories
 * KDB_MAX_UCHAR       the maximum value of unsigned char
 *
 * Following limits must be defined (in addition to limits mentioned
 * above for types):
 *
 * KDB_MAX_PATH_LENGTH the maximum length for a pathname
 *
 * In addition to the types the ... or va_list must be supported,
 * this is ISO C and should happen by including <stdarg.h>.
 *
 * Go ahead and write a #ifdef block for your operating system
 * when the POSIX defaults are not ok.
 */


#ifndef KDBOS_H
#define KDBOS_H

#ifdef __GNUC__
#define ELEKTRA_SENTINEL  __attribute__ ((sentinel))
#else
#define ELEKTRA_SENTINEL
#endif

#ifndef WIN32

/***************************************************
 *               Posix Compatible
 ***************************************************/

#ifndef __USE_POSIX
#define __USE_POSIX
#endif
#include <limits.h>

/* Conforming C++ programs are not allowed to
 * include inttypes.h*/
#include <inttypes.h>
#include <sys/types.h>


/**KDB_MAX_PATH_LENGTH will be the value for longest
 * possible filenames on the system.*/

/*Some systems have even longer pathnames*/
#ifdef PATH_MAX
#define KDB_MAX_PATH_LENGTH PATH_MAX
/*This value is garanteed on any Posixsystem*/
#elif defined __USE_POSIX
#define KDB_MAX_PATH_LENGTH _POSIX_PATH_MAX
#else
#define KDB_MAX_PATH_LENGTH 4096
#endif

/**Default Mode.
 * This mode will be used for new files*/
#define KDB_FILE_MODE 0664

/**Default directory mode.
 * This mode will be used for new directories.
 * Will be ORed together with KDB_FILE_MODE
 * to get the permissions of an directory.*/
#define KDB_DIR_MODE 0111



#else /* WIN32 */

/***************************************************
 *                 Windows
 ***************************************************/

/* Avoid the most crazy things */
#define NOMINMAX

# include <windows.h>
# include <limits.h>

# define usleep(x) Sleep(x)
# define ssize_t int
# define snprintf _snprintf

#define KDB_MAX_PATH_LENGTH 4096



#endif /* WIN32 */

/***************************************************
 *               For ANSI C systems
 ***************************************************/


/* Include essential headers used in kdb.h */
#include <stdarg.h>

/*Type to point to every position within the keyset
 * (note that for windows ssize_t is already redefined
 * as int) */
typedef ssize_t cursor_t;

/*Integer types*/
typedef int keyswitch_t;
typedef int option_t;

/**Separator for key names.
 * This character will be used to separate key names*/
#define KDB_PATH_SEPARATOR '/'
#define KDB_PATH_ESCAPE '\\'

#define KDB_MAX_UCHAR (UCHAR_MAX+1)



#endif /* KDBOS_H */
