#include "yajl.h"

#include <stdio.h>
#include <string.h>

#include <kdberrors.h>
#include <kdbconfig.h>
#include <yajl/yajl_parse.h>

#include "array.h"


/**
 @retval 0 if ksCurrent does not hold an array entry
 @retval 1 if the array entry will be used because its the first
 @retval 2 if a new array entry was created
 @retval -1 error in snprintf
 */
static int elektraYajlIncrementArrayEntry(KeySet * ks)
{
	Key * current = ksCurrent(ks);
	const char * baseName = keyBaseName(current);

	if (baseName && *baseName == '#')
	{
		if (!strcmp(baseName, "###empty_array"))
		{
			// we have a new array entry, just use it
			keySetBaseName (current, "#0");
			return 1;
		}
		else
		{
			// we are in an array
			Key * newKey = keyNew (keyName(current), KEY_END);
			elektraArrayIncName(newKey);
			ksAppendKey(ks, newKey);
			return 2;
		}
	}
	else
	{
		// previous entry indicates this is not an array
		return 0;
	}
}

static int elektraYajlParseNull(void *ctx)
{
	KeySet *ks = (KeySet*) ctx;
	elektraYajlIncrementArrayEntry(ks);

	Key * current = ksCurrent(ks);

	keySetBinary(current, NULL, 0);

#ifdef ELEKTRA_YAJL_VERBOSE
	printf ("elektraYajlParseNull\n");
#endif

	return 1;
}

static int elektraYajlParseBoolean(void *ctx, int boolean)
{
	KeySet *ks = (KeySet*) ctx;
	elektraYajlIncrementArrayEntry(ks);

	Key * current = ksCurrent(ks);

	if (boolean == 1)
	{
		keySetString(current, "true");
	}
	else
	{
		keySetString(current, "false");
	}
	keySetMeta(current, "type", "boolean");

#ifdef ELEKTRA_YAJL_VERBOSE
	printf ("elektraYajlParseBoolean %d\n", boolean);
#endif

	return 1;
}

static int elektraYajlParseNumber(void *ctx, const char *stringVal,
			yajl_size_type stringLen)
{
	KeySet *ks = (KeySet*) ctx;
	elektraYajlIncrementArrayEntry(ks);

	Key *current = ksCurrent(ks);

	unsigned char delim = stringVal[stringLen];
	char * stringValue = (char*)stringVal;
	stringValue[stringLen] = '\0';

#ifdef ELEKTRA_YAJL_VERBOSE
	printf ("elektraYajlParseNumber %s %d\n", stringVal, stringLen);
#endif

	keySetString(current, stringVal);
	keySetMeta(current, "type", "number");

	// restore old character in buffer
	stringValue[stringLen] = delim;

	return 1;
}

static int elektraYajlParseString(void *ctx, const unsigned char *stringVal,
			yajl_size_type stringLen)
{
	KeySet *ks = (KeySet*) ctx;
	elektraYajlIncrementArrayEntry(ks);

	Key *current = ksCurrent(ks);

	unsigned char delim = stringVal[stringLen];
	char * stringValue = (char*)stringVal;
	stringValue[stringLen] = '\0';

#ifdef ELEKTRA_YAJL_VERBOSE
	printf ("elektraYajlParseString %s %d\n", stringVal, stringLen);
#endif

	keySetString(current, stringValue);

	// restore old character in buffer
	stringValue[stringLen] = delim;
	return 1;
}

static int elektraYajlParseMapKey(void *ctx, const unsigned char * stringVal,
			 yajl_size_type stringLen)
{
	KeySet *ks = (KeySet*) ctx;
	elektraYajlIncrementArrayEntry(ks);

	Key *currentKey = ksCurrent(ks);

	unsigned char delim = stringVal[stringLen];
	char * stringValue = (char*)stringVal;
	stringValue[stringLen] = '\0';

#ifdef ELEKTRA_YAJL_VERBOSE
	printf ("elektraYajlParseMapKey stringValue: %s currentKey: %s\n", stringValue,
			keyName(currentKey));
#endif
	if (currentKey && !strcmp(keyBaseName(currentKey), "___empty_map"))
	{
		// now we know the name of the object
		keySetBaseName(currentKey, stringValue);
	}
	else
	{
		// we entered a new pair (inside the previous object)
		Key * newKey = keyNew (keyName(currentKey), KEY_END);
		keySetBaseName(newKey, stringValue);
		ksAppendKey(ks, newKey);
	}

	// restore old character in buffer
	stringValue[stringLen] = delim;

	return 1;
}

static int elektraYajlParseStartMap(void *ctx)
{
	KeySet *ks = (KeySet*) ctx;
	elektraYajlIncrementArrayEntry(ks);

	Key *currentKey = ksCurrent(ks);

	Key * newKey = keyNew (keyName(currentKey), KEY_END);
	// add a pseudo element for empty map
	keyAddBaseName(newKey, "___empty_map");
	ksAppendKey(ks, newKey);

#ifdef ELEKTRA_YAJL_VERBOSE
	printf ("elektraYajlParseStartMap with new key %s\n", keyName(newKey));
#endif

	return 1;
}

static int elektraYajlParseEnd(void *ctx)
{
	KeySet *ks = (KeySet*) ctx;
	Key *currentKey = ksCurrent(ks);

	Key * lookupKey = keyNew (keyName(currentKey), KEY_END);
	keySetBaseName(lookupKey, ""); // remove current baseName

	// lets point current to the correct place
	Key * foundKey = ksLookup(ks, lookupKey, 0);

#ifdef ELEKTRA_YAJL_VERBOSE
	if (foundKey)
	{
		printf ("elektraYajlParseEnd %s\n", keyName(foundKey));
	}
	else
	{
		printf ("elektraYajlParseEnd did not find key!\n");
	}
#else
	(void)foundKey; // foundKey is not used, but lookup is needed
#endif

	keyDel (lookupKey);

	return 1;
}

static int elektraYajlParseStartArray(void *ctx)
{
	KeySet *ks = (KeySet*) ctx;
	elektraYajlIncrementArrayEntry(ks);

	Key *currentKey = ksCurrent(ks);

	Key * newKey = keyNew (keyName(currentKey), KEY_END);
	// add a pseudo element for empty array
	keyAddBaseName(newKey, "###empty_array");
	ksAppendKey(ks, newKey);

#ifdef ELEKTRA_YAJL_VERBOSE
	printf ("elektraYajlParseStartArray with new key %s\n", keyName(newKey));
#endif

	return 1;
}

int elektraYajlGet(Plugin *handle ELEKTRA_UNUSED, KeySet *returned,
		Key *parentKey)
{
	if (!strcmp (keyName(parentKey), "system/elektra/modules/yajl"))
	{
		KeySet *moduleConfig =
#include "contract.h"
		ksAppend(returned, moduleConfig);
		ksDel(moduleConfig);
		return 1;
	}

	yajl_callbacks callbacks = {
		elektraYajlParseNull,
		elektraYajlParseBoolean,
		NULL,
		NULL,
		elektraYajlParseNumber,
		elektraYajlParseString,
		elektraYajlParseStartMap,
		elektraYajlParseMapKey,
		elektraYajlParseEnd,
		elektraYajlParseStartArray,
		elektraYajlParseEnd
	};

	ksAppendKey(returned, keyNew(keyName((parentKey)), KEY_END));

#if YAJL_MAJOR == 1
	yajl_parser_config cfg = { 1, 1 };
	yajl_handle hand = yajl_alloc(&callbacks, &cfg, NULL, returned);
#else
	yajl_handle hand = yajl_alloc(&callbacks, NULL, returned);
	yajl_config(hand, yajl_allow_comments, 1);
#endif

	unsigned char fileData[65536];
	int done = 0;
	FILE * fileHandle = fopen(keyString(parentKey), "r");
	if (!fileHandle)
	{
		// ELEKTRA_SET_ERROR(75, parentKey, keyString(parentKey));
		// return -1;
		return 0;
	}

	while (!done)
	{
		yajl_size_type rd = fread(	(void *) fileData, 1,
					sizeof(fileData) - 1,
					fileHandle);
		if (rd == 0)
		{
			if (!feof(fileHandle))
			{
				ELEKTRA_SET_ERROR(76, parentKey,
						keyString(parentKey));
				fclose (fileHandle);
				return -1;
			}
			done = 1;
		}
		fileData[rd] = 0;

		yajl_status stat;
		if (done)
		{
#if YAJL_MAJOR == 1
			stat = yajl_parse_complete(hand);
#else
			stat = yajl_complete_parse(hand);
#endif
		}
		else
		{
			stat = yajl_parse(hand, fileData, rd);
		}

		if (stat != yajl_status_ok
#if YAJL_MAJOR == 1
			&& stat != yajl_status_insufficient_data
#endif
			)
		{
			unsigned char * str = yajl_get_error(hand, 1,
					fileData, rd);
			ELEKTRA_SET_ERROR(77, parentKey, (char*)str);
			yajl_free_error(hand, str);
			yajl_free(hand);
			fclose (fileHandle);

			return -1;
		}
	}

	yajl_free(hand);
	fclose(fileHandle);

	return 1; /* success */
}