/***************************************************************************
                     sync.c  -  Skeleton of a plugin
                             -------------------
    begin                : Fri May 21 2010
    copyright            : (C) 2010 by Markus Raab
    email                : elektra@markus-raab.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This is the skeleton of the methods you'll have to implement in order *
 *   to provide a valid plugin.                                            *
 *   Simple fill the empty functions with your code and you are            *
 *   ready to go.                                                          *
 *                                                                         *
 ***************************************************************************/


#ifndef HAVE_KDBCONFIG
# include "kdbconfig.h"
#endif

#include "sync.h"

#include <kdberrors.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define ERROR_SIZE 1024


int elektraSyncGet(Plugin *handle ELEKTRA_UNUSED, KeySet *returned ELEKTRA_UNUSED, Key *parentKey ELEKTRA_UNUSED)
{
	if (!elektraStrCmp(keyName(parentKey), "system/elektra/modules/sync"))
	{
		KeySet *contract = ksNew (30,
		keyNew ("system/elektra/modules/sync",
			KEY_VALUE, "dbus plugin waits for your orders", KEY_END),
		keyNew ("system/elektra/modules/sync/exports", KEY_END),
		keyNew ("system/elektra/modules/sync/exports/get",
			KEY_FUNC, elektraSyncGet, KEY_END),
		keyNew ("system/elektra/modules/sync/exports/set",
			KEY_FUNC, elektraSyncSet, KEY_END),
#include ELEKTRA_README(sync)
		keyNew ("system/elektra/modules/sync/infos/version",
			KEY_VALUE, PLUGINVERSION, KEY_END),
		KS_END);
		ksAppend (returned, contract);
		ksDel (contract);

		return 1; /* success */
	}
	/* get all keys */

	return 1; /* success */
}

int elektraSyncSet(Plugin *handle ELEKTRA_UNUSED, KeySet *returned ELEKTRA_UNUSED, Key *parentKey)
{
	/* set all keys */
	const char *configFile = keyString(parentKey);
	int fd = open(configFile, O_RDWR);
	if (fd == -1)
	{
		char buffer[ERROR_SIZE];
		strerror_r(errno, buffer, ERROR_SIZE);
		ELEKTRA_SET_ERRORF(89, parentKey,
			"Could not open config file %s because %s",
			configFile, buffer);
		return -1;
	}
	if (fsync(fd) == -1)
	{
		char buffer[ERROR_SIZE];
		strerror_r(errno, buffer, ERROR_SIZE);
		ELEKTRA_SET_ERRORF(89, parentKey,
			"Could not fsync config file %s because %s",
			configFile, buffer);
		close(fd);
		return -1;
	}
	close(fd);

	return 1; /* success */
}

Plugin *ELEKTRA_PLUGIN_EXPORT(sync)
{
	return elektraPluginExport("sync",
		ELEKTRA_PLUGIN_GET,	&elektraSyncGet,
		ELEKTRA_PLUGIN_SET,	&elektraSyncSet,
		ELEKTRA_PLUGIN_END);
}

