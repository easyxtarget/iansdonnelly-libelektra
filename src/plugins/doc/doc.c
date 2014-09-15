/***************************************************************************
            doc.c  - Documentation on how to write plugins
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

#include "doc.h"

//! [plugin include]
#include <kdbplugin.h>
//! [plugin include]
//
//! [plugin errors include]
#include <kdberrors.h>
//! [plugin errors include]

#include <stdlib.h>
#include <stdio.h>

#ifndef HAVE_KDBCONFIG
# include "kdbconfig.h"
#endif


//! [global data]
typedef struct { int global; } GlobalData;
//! [global data]


#define DOC_PLUGIN_NAME "doc"
#define DOC_PLUGIN_VERSION "1.0.0"


//! [doc open]
int elektraDocOpen(Plugin *handle, Key *warningsKey ELEKTRA_UNUSED)
{
	GlobalData *data;
	KeySet *config = elektraPluginGetConfig(handle);
	Key * kg = ksLookupByName(config, "/global", 0);

	data=malloc(sizeof(GlobalData));
	data->global = 0;
	if (kg) data->global = atoi(keyString(kg));
	elektraPluginSetData(handle,data);
//! [doc open]

//! [doc module]
	if (ksLookupByName(config, "/module", 0))
	{
		return 0;
	}
	// do some setup that will fail without configuration
//! [doc module]

	return 0; /* success */
}


//! [doc close]
int elektraDocClose(Plugin *handle, Key *warningsKey ELEKTRA_UNUSED)
{
	free (elektraPluginGetData(handle));

	return 0; /* success */
}
//! [doc close]

static int parseKey(FILE *fp ELEKTRA_UNUSED, char **key ELEKTRA_UNUSED, char **value ELEKTRA_UNUSED)
{
	return 0;
}

static void doAction(Key *k ELEKTRA_UNUSED)
{
}

//![get contract]
int elektraDocGet(Plugin *plugin ELEKTRA_UNUSED, KeySet *returned, Key *parentKey)
{
	if (!strcmp(keyName(parentKey), "system/elektra/modules/doc"))
	{
		KeySet *contract = ksNew (30,
		keyNew ("system/elektra/modules/doc",
			KEY_VALUE, "dbus plugin waits for your orders", KEY_END),
		keyNew ("system/elektra/modules/doc/exports", KEY_END),
		keyNew ("system/elektra/modules/doc/exports/open",
			KEY_FUNC, elektraDocOpen, KEY_END),
		keyNew ("system/elektra/modules/doc/exports/close",
			KEY_FUNC, elektraDocClose, KEY_END),
		keyNew ("system/elektra/modules/doc/exports/get",
			KEY_FUNC, elektraDocGet, KEY_END),
		keyNew ("system/elektra/modules/doc/exports/set",
			KEY_FUNC, elektraDocSet, KEY_END),
		keyNew ("system/elektra/modules/doc/exports/error",
			KEY_FUNC, elektraDocError, KEY_END),
#include ELEKTRA_README(doc)
		keyNew ("system/elektra/modules/doc/infos/version",
			KEY_VALUE, PLUGINVERSION, KEY_END),
		KS_END);
		ksAppend (returned, contract);
		ksDel (contract);

		return 1; /* success */
	}
//![get contract]

//![get storage]
	FILE *fp = fopen (keyString(parentKey), "r");
	char *key;
	char *value;

	while (parseKey(fp, &key, &value) >= 1)
	{
		Key *read = keyNew(0);
		if (keySetName(read, key) == -1)
		{
			fclose (fp);
			keyDel (read);
			ELEKTRA_SET_ERROR(59, parentKey, key);
			return -1;
		}
		keySetString(read, value);

		ksAppendKey (returned, read);
		free (key);
		free (value);
	}

	if (feof(fp) == 0)
	{
		fclose (fp);
		ELEKTRA_SET_ERROR(60, parentKey, "not at the end of file");
		return -1;
	}

	fclose (fp);
//![get storage]


//![get filter]
	Key *k;
	ksRewind (returned);
	while ((k = ksNext (returned)) != 0)
	{
		doAction(k);
	}

	return 1; // success
}
//![get filter]

int elektraDocSet(Plugin *handle ELEKTRA_UNUSED, KeySet *returned ELEKTRA_UNUSED, Key *parentKey ELEKTRA_UNUSED)
{
	ssize_t nr_keys = 0;
	/* set all keys below parentKey and count them with nr_keys */

	return nr_keys;
}

int elektraDocError(Plugin *handle ELEKTRA_UNUSED, KeySet *returned ELEKTRA_UNUSED, Key *parentKey ELEKTRA_UNUSED)
{
	return 0;
}

static Plugin *findPlugin(KDB *handle ELEKTRA_UNUSED)
{
	return 0;
}

static void saveToDisc (Key *k ELEKTRA_UNUSED)
{
}

//![set full]
static void usercode (KDB *handle, KeySet *keyset, Key *key)
{
	// some more user code
	keySetString (key, "mycomment"); // the user changes the key
	ksAppendKey(keyset, key); // append the key to the keyset
	kdbSet (handle, keyset, 0); // and syncs it to disc
}

// so now kdbSet is called
int elektraKdbSet(KDB *handle, KeySet *keyset, Key *parentKey)
{
	int ret = 0;
	// find appropriate plugin and then call it:
	Plugin *plugin = findPlugin(handle);
	ret = elektraDocSet (plugin, keyset, parentKey);
	// the keyset with the key (and others for this plugin)
	// will be passed to this function
	return ret;
}

// so now elektraPluginSet(), which is the function described here,
// is called:
int elektraPluginSet(Plugin *plugin ELEKTRA_UNUSED, KeySet *returned, Key *parentKey ELEKTRA_UNUSED)
{
	// the task of elektraPluginSet is now to store the keys
	Key *k;
	ksRewind (returned);
	while ((k = ksNext (returned)) != 0)
	{
		saveToDisc (k);
	}

	return 1; /* success */
}
//![set full]


//![export]
Plugin *ELEKTRA_PLUGIN_EXPORT(doc)
{
	return elektraPluginExport(DOC_PLUGIN_NAME,
		ELEKTRA_PLUGIN_OPEN,	&elektraDocOpen,
		ELEKTRA_PLUGIN_CLOSE,	&elektraDocClose,
		ELEKTRA_PLUGIN_GET,	&elektraDocGet,
		ELEKTRA_PLUGIN_SET,	&elektraDocSet,
		ELEKTRA_PLUGIN_ERROR,	&elektraDocError,
		ELEKTRA_PLUGIN_END);
}
//![export]

/**
 * @}
 */
