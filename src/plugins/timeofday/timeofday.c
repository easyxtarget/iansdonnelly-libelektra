/***************************************************************************
          timeofday.c  -  Skeleton of a plugin to be copied
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
 *   to provide libelektra.so a valid plugin.                             *
 *   Simple fill the empty _timeofday functions with your code and you are   *
 *   ready to go.                                                          *
 *                                                                         *
 ***************************************************************************/


#ifndef HAVE_KDBCONFIG
# include "kdbconfig.h"
#endif

#include "timeofday.h"

static inline void timeofday(char *t, struct timeval *start, struct timeval *now)
{
	struct timeval tv;

	tv.tv_sec = now->tv_sec - start->tv_sec;
	if ((tv.tv_usec = now->tv_usec - start->tv_usec) < 0)
	{
		tv.tv_sec --;
		tv.tv_usec += 1000000;
	}

	for (int i=9; i>=4; --i)
	{
		t[i] = tv.tv_usec %10 + '0';
		tv.tv_usec /= 10;
	}
	for (int i=3; i>=0; --i)
	{
		t[i] = tv.tv_sec %10 + '0';
		tv.tv_sec /= 10;
	}
	t[10] = 0;
}

const char * elektraTimeofdayHelper (char *t, TimeofdayInfo *ti)
{
	struct timeval now;
	gettimeofday(&now, 0);
	timeofday (t, &ti->start, &now);
	t[10] = '\t';
	t[11] = 'd';
	t[12] = 'i';
	t[13] = '\t';
	timeofday(&t[14], &ti->last, &now);
	ti->last = now;

	return t;
}

int elektraTimeofdayOpen(Plugin *handle, Key *parentKey ELEKTRA_UNUSED)
{
	TimeofdayInfo *ti = calloc(1, sizeof (TimeofdayInfo));
	// char t[24];

	elektraPluginSetData(handle, ti);

	// init time
	gettimeofday(&ti->start, 0);
	ti->last = ti->start;

	// fprintf(stderr, "open\t%s\n", elektraTimeofdayHelper (t, ti));

	return 0; /* success */
}

int elektraTimeofdayClose(Plugin *handle, Key *parentKey ELEKTRA_UNUSED)
{
	// char t[24];
	// TimeofdayInfo *ti = elektraPluginGetData(handle);

	// fprintf(stderr, "close\t%s\n", elektraTimeofdayHelper (t, ti));

	/* How weird is that??
	   ti gets modified after elektraTimeofdayHelper even though
	   the pointer to it is not even passed and it is valgrind
	   clean?
           Fixed by using fti */
	TimeofdayInfo *fti = elektraPluginGetData(handle);
	free(fti);

	return 0; /* success */
}

int elektraTimeofdayGet(Plugin *handle, KeySet *returned, Key *parentKey)
{
	char t[24];
	TimeofdayInfo *ti = elektraPluginGetData(handle);
	const char *position = "get";

	ti->nrset = 0;
	++ ti->nrget;
	if (ti->nrget == 1) position = "pregetstorage";
	else if (ti->nrget == 2)
	{
		ti->nrget = 0;
		position = "postgetstorage";
	}

	fprintf(stderr, "get\t%s\tpos\t%s\n", elektraTimeofdayHelper (t, ti), position);

	Key *root = keyNew("system/elektra/modules/timeofday", KEY_END);
	if (keyRel (root, parentKey) >= 0)
	{
		KeySet *pluginConfig = ksNew (30,
			keyNew ("system/elektra/modules/timeofday",
				KEY_VALUE, "timeofday plugin waits for your orders", KEY_END),
			keyNew ("system/elektra/modules/timeofday/exports", KEY_END),
			keyNew ("system/elektra/modules/timeofday/exports/open",
				KEY_FUNC, elektraTimeofdayOpen, KEY_END),
			keyNew ("system/elektra/modules/timeofday/exports/close",
				KEY_FUNC, elektraTimeofdayClose, KEY_END),
			keyNew ("system/elektra/modules/timeofday/exports/get",
				KEY_FUNC, elektraTimeofdayGet, KEY_END),
			keyNew ("system/elektra/modules/timeofday/exports/set",
				KEY_FUNC, elektraTimeofdaySet, KEY_END),
			keyNew ("system/elektra/modules/timeofday/exports/error",
				KEY_FUNC, elektraTimeofdayError, KEY_END),
#include "readme_timeofday.c"
			keyNew ("system/elektra/modules/timeofday/infos/version",
				KEY_VALUE, PLUGINVERSION, KEY_END),
			KS_END);
		ksAppend (returned, pluginConfig);
		ksDel (pluginConfig);

		fprintf(stderr, "get\t%s\tpos\t%s\n", elektraTimeofdayHelper (t, ti), "postmodulesconf");
	}

	keyDel (root);

	return 1;
}

int elektraTimeofdaySet(Plugin *handle, KeySet *returned ELEKTRA_UNUSED, Key *parentKey ELEKTRA_UNUSED)
{
	char t[24];
	TimeofdayInfo *ti = elektraPluginGetData(handle);
	const char *position = "set";

	ti->nrget = 0;
	++ ti->nrset;
	if (ti->nrset == 1) position = "presetstorage";
	else if (ti->nrset == 2) position = "precommit";
	else if (ti->nrset == 3)
	{
		ti->nrset = 0;
		position = "postcommit";
	}

	fprintf(stderr, "set\t%s\tpos\t%s\n", elektraTimeofdayHelper (t, ti), position);

	return 1;
}

int elektraTimeofdayError(Plugin *handle, KeySet *returned ELEKTRA_UNUSED, Key *parentKey ELEKTRA_UNUSED)
{
	char t[24];
	TimeofdayInfo *ti = elektraPluginGetData(handle);
	const char *position = "error";

	ti->nrset = 0;
	ti->nrget = 0;
	++ ti->nrerr;
	if (ti->nrerr == 1) position = "prerollback";
	else if (ti->nrerr == 2)
	{
		ti->nrerr = 0;
		position = "postrollback";
	}

	fprintf(stderr, "err\t%s\tpos\t%s\n", elektraTimeofdayHelper (t, ti), position);

	return 1;
}

Plugin *ELEKTRA_PLUGIN_EXPORT(timeofday)
{
	return elektraPluginExport(BACKENDNAME,
		ELEKTRA_PLUGIN_OPEN,	&elektraTimeofdayOpen,
		ELEKTRA_PLUGIN_CLOSE,	&elektraTimeofdayClose,
		ELEKTRA_PLUGIN_GET,	&elektraTimeofdayGet,
		ELEKTRA_PLUGIN_SET,	&elektraTimeofdaySet,
		ELEKTRA_PLUGIN_ERROR,	&elektraTimeofdayError,
		ELEKTRA_PLUGIN_END);
}

