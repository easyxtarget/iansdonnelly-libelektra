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
#include <stddef.h>
#include <string.h>

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
			keyNew ("system/elektra/modules/line/infos/version",
				KEY_VALUE, PLUGINVERSION, KEY_END),
			KS_END);
		ksAppend (returned, moduleConfig);
		ksDel (moduleConfig);
		return 1;
	}
	printf("parentKey Name == %s\n", keyName(parentKey));
	int n;
	char *value;
	char *key;
	Key *read;
	int i = 0;
	size_t len = 0;
	size_t numberSize;
	size_t stringSize;
	FILE *fp = fopen (keyString(parentKey), "r");
	if (!fp)
	{
		// ELEKTRA_SET_ERROR(74, parentKey, keyString(parentKey));
		// return -1;
		return 0; // we just ignore if we could not open file
	}
	//Read in each line
	while ((n = getline (&value, &len, fp)) != -1)
	{
		i++;
		numberSize = snprintf(0, 0, "%d", i);
		stringSize = sizeof("line") + numberSize + 1;
		key = malloc(stringSize);
		snprintf (key, stringSize, "line%d", i);
		printf("key = '%s'\n", key);
		printf("value = '%s'\n", value);
		read = keyDup(parentKey);
		if (keyAddBaseName(read, key) == -1)
		{
			fclose (fp);
			keyDel (read);
			ELEKTRA_SET_ERROR(59, parentKey, key);
			return -1;
		}
		printf("read name = %s\n", keyName(read));
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
