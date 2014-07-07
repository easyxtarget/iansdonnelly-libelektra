/**
 * \file
 *
 * \brief A plugin that reads configuration files and saves keys on a line by line basis *
 * \copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#ifndef HAVE_KDBCONFIG
#include "kdbconfig.h"
#endif

#include "line.h"

#include <kdberrors.h>

#include <stdio.h>
#include <stdlib.h>

// 4.7.2 supports %ms but yields warning using -Wformat together with
// -ansi -pedantic
// warning: ISO C does not support the 'm' scanf flag
#define GCC_VERSION (__GNUC__ * 10000 \
                              + __GNUC_MINOR__ * 100 \
                              + __GNUC_PATCHLEVEL__)
#if  GCC_VERSION < 40800
# pragma GCC diagnostic ignored "-Wformat"
#endif

int elektraLineGet(Plugin *handle ELEKTRA_UNUSED, KeySet *returned, Key *parentKey)
{
	/* get all keys */

	if (!strcmp (keyName(parentKey), "system/elektra/modules/line"))
	{
		KeySet *moduleConfig = ksNew (30,
			keyNew ("system/elektra/modules/line",
				KEY_VALUE, "line plugin waits for your orders", KEY_END),
			keyNew ("system/elektra/modules/line/exports", KEY_END),
			keyNew ("system/elektra/modules/line/exports/get",
				KEY_FUNC, elektraLineGet, KEY_END),
			keyNew ("system/elektra/modules/line/exports/set",
				KEY_FUNC, elektraLineSet, KEY_END),
			keyNew ("system/elektra/modules/line/infos",
				KEY_VALUE, "All information you want to know", KEY_END),
			keyNew ("system/elektra/modules/line/infos/author",
				KEY_VALUE, "Ian Donnelly <ian.s.donnelly@gmail.com>", KEY_END),
			keyNew ("system/elektra/modules/line/infos/licence",
				KEY_VALUE, "BSD", KEY_END),
			keyNew ("system/elektra/modules/line/infos/description",
				KEY_VALUE, "Very simple storage which writes out line by line", KEY_END),
			keyNew ("system/elektra/modules/line/infos/provides",
				KEY_VALUE, "storage", KEY_END),
			keyNew ("system/elektra/modules/line/infos/placements",
				KEY_VALUE, "getstorage setstorage", KEY_END),
			keyNew ("system/elektra/modules/line/infos/needs",
				KEY_VALUE, "code null", KEY_END),
			keyNew ("system/elektra/modules/line/infos/version",
				KEY_VALUE, PLUGINVERSION, KEY_END),
			keyNew ("system/elektra/modules/line/config/needs",
				KEY_VALUE, "the needed configuration to work in a backend", KEY_END),
			keyNew ("system/elektra/modules/line/config/needs/chars",
				KEY_VALUE, "Characters needed", KEY_END),
			keyNew ("system/elektra/modules/line/config/needs/chars/20",
				KEY_VALUE, "61", KEY_END), // space -> a
			keyNew ("system/elektra/modules/line/config/needs/chars/23",
				KEY_VALUE, "62", KEY_END), // # -> b
			keyNew ("system/elektra/modules/line/config/needs/chars/25",
				KEY_VALUE, "63", KEY_END), // % -> c (escape character)
			keyNew ("system/elektra/modules/line/config/needs/chars/3B",
				KEY_VALUE, "64", KEY_END), // ; -> d
			keyNew ("system/elektra/modules/line/config/needs/chars/3D",
				KEY_VALUE, "65", KEY_END), // = -> e
			keyNew ("system/elektra/modules/line/config/needs/chars/5C",
				KEY_VALUE, "66", KEY_END), // \\ -> f
			keyNew ("system/elektra/modules/line/config/needs/chars/0A",
				KEY_VALUE, "67", KEY_END), // enter (NL) -> g
			keyNew ("system/elektra/modules/line/config/needs/chars/0D",
				KEY_VALUE, "68", KEY_END), // CR -> h
			keyNew ("system/elektra/modules/line/config/needs/escape",
				KEY_VALUE, "25", KEY_END),
			KS_END);
		ksAppend (returned, moduleConfig);
		ksDel (moduleConfig);
		return 1;
	}

	int n;
	char *key;
	char *value;
	int i = 0;
	char *i_string;
	FILE *fp = fopen (keyString(parentKey), "r");
	if (!fp)
	{
		printf("Could not open file\n");
		// ELEKTRA_SET_ERROR(74, parentKey, keyString(parentKey));
		// return -1;
		return 0; // we just ignore if we could not open file
	}
	printf("Opened file successfully\n");
	while ((n = fscanf (fp, "%ms\n", &value)) >= 1)
	{
		i++;
		key = "line";
		sprintf(i_string, "%d", i);
		strcat(key, i_string);
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

	return 1; /* success */
}

int elektraLineSet(Plugin *handle ELEKTRA_UNUSED, KeySet *returned, Key *parentKey)
{
	/* set all keys */

	FILE *fp = fopen(keyString(parentKey), "w");
	if (!fp)
	{
		ELEKTRA_SET_ERROR(74, parentKey, keyString(parentKey));
		return -1;
	}

	Key *cur;
	ksRewind (returned);
	while ((cur = ksNext(returned)) != 0)
	{
		fprintf (fp, "%s\n", keyString(cur));
	}

	fclose (fp);

	return 1; /* success */
}

Plugin *ELEKTRA_PLUGIN_EXPORT(line)
{
	return elektraPluginExport("line",
		ELEKTRA_PLUGIN_GET,	&elektraLineGet,
		ELEKTRA_PLUGIN_SET,	&elektraLineSet,
		ELEKTRA_PLUGIN_END);
}
