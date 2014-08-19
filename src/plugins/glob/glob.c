/**
 * \file
 *
 * \brief A plugin that converts keys to metakeys and vice versa
 *
 * \copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#include "glob.h"

#ifndef HAVE_KDBCONFIG
# include "kdbconfig.h"
#endif

int elektraGlobMatch(Key *key, const Key *match, int globFlags)
{
	if (!fnmatch (keyString(match), keyName(key),
			globFlags))
	{
		keyCopyAllMeta(key, match);
	}
	return 0;
}

enum GlobDirection {
	GET,
	SET,
};

static const char *getGlobFlags (KeySet* keys, Key *globKey)
{
	Key *flagKey = keyDup (globKey);
	keyAddBaseName(flagKey, "flags");
	Key *flagResult = ksLookup (keys, flagKey, KDB_O_NONE);
	keyDel(flagKey);

	if (flagResult)
	{
		return keyString (flagResult);
	}

	return 0;
}

static KeySet* getGlobKeys(Key* parentKey, KeySet* keys, enum GlobDirection direction)
{
	KeySet* glob = ksNew(0, KS_END);
	Key* k = 0;
	size_t parentsize = keyGetNameSize (parentKey);

	Key *userGlobConfig = 0;
	Key *systemGlobConfig = 0;
	Key *userDirGlobConfig = 0;
	Key *systemDirGlobConfig = 0;

	userGlobConfig = keyNew ("user/glob", KEY_END);
	systemGlobConfig = keyNew ("system/glob", KEY_END);
	switch(direction)
	{
	case GET:
		userDirGlobConfig = keyNew ("user/glob/get", KEY_END);
		systemDirGlobConfig = keyNew ("system/glob/get", KEY_END);
		break;
	case SET:
		userDirGlobConfig = keyNew ("user/glob/set", KEY_END);
		systemDirGlobConfig = keyNew ("system/glob/set", KEY_END);
		break;
	}

	while ((k = ksNext (keys)) != 0)
	{
		/* use only glob keys for the current direction */
		if (keyIsDirectBelow(userGlobConfig, k) || keyIsDirectBelow(systemGlobConfig, k) ||
				keyIsDirectBelow (userDirGlobConfig, k) || keyIsDirectBelow (systemDirGlobConfig, k))
		{
			keySetMeta (k, "flags", getGlobFlags (keys, k));

			/* Look if we have a string */
			size_t valsize = keyGetValueSize (k);
			if (valsize < 2) continue;

			/* We now know we want that key.
			 Dup it to not change the configuration. */
			Key* ins = keyDup (k);
			/* Now look if we want cascading for the key */
			if (keyString (k)[0] == '/')
			{
				char* newstring = malloc (valsize + parentsize);
				strcpy (newstring, keyName (parentKey));
				strcat (newstring, keyString (k));
				keySetString (ins, newstring);
				free (newstring);
			}
			ksAppendKey (glob, ins);
		}
	}

	keyDel (userGlobConfig);
	keyDel (systemGlobConfig);
	keyDel (userDirGlobConfig);
	keyDel (systemDirGlobConfig);

	return glob;
}

static void applyGlob(KeySet* returned, KeySet* glob)
{
	Key* cur;
	ksRewind (returned);
	while ((cur = ksNext (returned)) != 0)
	{
		Key* match;
		ksRewind (glob);
		while ((match = ksNext (glob)) != 0)
		{
			const Key *flagKey = keyGetMeta(match, "flags");

			if (flagKey)
			{
				char *end;
				int flags = strtol (keyString(flagKey), &end, 10);
				if (!*end)
				{
					elektraGlobMatch (cur, match, flags);
					continue;
				}
			}

			/* if no flags were provided, default to FNM_PATHNAME behaviour */
			elektraGlobMatch (cur, match, FNM_PATHNAME);
		}
	}
}

int elektraGlobOpen(Plugin *handle ELEKTRA_UNUSED, Key *parentKey ELEKTRA_UNUSED)
{
	/* plugin initialization logic should be here */
	/* TODO: name of parentKey is not set...*/
	/* So rewriting cannot happen here (is in elektraGlobSet */

	return 1; /* success */
}

int elektraGlobClose(Plugin *handle ELEKTRA_UNUSED, Key *errorKey ELEKTRA_UNUSED)
{
	/* free all plugin resources and shut it down */

	KeySet *keys = elektraPluginGetData(handle);
	ksDel (keys);

	return 1; /* success */
}



int elektraGlobGet(Plugin *handle ELEKTRA_UNUSED, KeySet *returned, Key *parentKey ELEKTRA_UNUSED)
{
	if (!strcmp (keyName(parentKey), "system/elektra/modules/glob"))
	{
		// TODO: improve plugin contract
		KeySet *config =
				#include "contract.h"
		ksAppend (returned, config);
		return 1;
	}

	KeySet *keys = elektraPluginGetConfig(handle);
	ksRewind (keys);

	KeySet* glob = getGlobKeys (parentKey, keys, GET);
	applyGlob (returned, glob);

	ksDel (glob);

	return 1; /* success */
}



int elektraGlobSet(Plugin *handle, KeySet *returned, Key *parentKey)
{
	KeySet *keys = elektraPluginGetConfig(handle);
	ksRewind (keys);

	KeySet* glob = getGlobKeys (parentKey, keys, SET);
	applyGlob (returned, glob);

	ksDel (glob);

	return 1; /* success */
}

Plugin *ELEKTRA_PLUGIN_EXPORT(glob)
{
	return elektraPluginExport("glob",
		ELEKTRA_PLUGIN_OPEN,	&elektraGlobOpen,
		ELEKTRA_PLUGIN_CLOSE,	&elektraGlobClose,
		ELEKTRA_PLUGIN_GET,	&elektraGlobGet,
		ELEKTRA_PLUGIN_SET,	&elektraGlobSet,
		ELEKTRA_PLUGIN_END);
}

