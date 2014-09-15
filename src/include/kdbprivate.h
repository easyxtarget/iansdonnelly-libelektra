/***************************************************************************
      kdbprivate.h  -  Private declarations

                           -------------------
 *  begin                : Wed 19 May, 2010
 *  copyright            : (C) 2010 by Markus Raab
 *  email                : elektra@markus-raab.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

#ifndef KDBPRIVATE_H
#define KDBPRIVATE_H

#include <kdb.h>
#include <kdbhelper.h>
#include <kdbconfig.h>
#include <kdbplugin.h>
#include <kdbproposal.h>
#include <kdbextension.h>

#include <limits.h>

/** The minimal allocation size of a keyset inclusive
    NULL byte. ksGetAlloc() will return one less because
    it says how much can actually be stored.*/
#define KEYSET_SIZE 16

/** How many plugins can exist in an backend. */
#define NR_OF_PLUGINS 10

/** The index of the commit plugin */
#define COMMIT_PLUGIN 7

/** The index of the storage plugin */
#define STORAGE_PLUGIN 5

/** The index of the resolver plugin */
#define RESOLVER_PLUGIN 0

/** Trie optimization */
#define APPROXIMATE_NR_OF_BACKENDS 16

/**The maximum of how many characters an integer
  needs as decimal number.*/
#define MAX_LEN_INT 31

/**Backend mounting information.
 *
 * This key directory tells you where each backend is mounted
 * to which mountpoint. */
#define KDB_KEY_MOUNTPOINTS      "system/elektra/mountpoints"

#if DEBUG
# include <stdio.h>
# define ELEKTRA_PRINT_DEBUG(text) printf("%s:%d: %s\n", __FILE__, __LINE__ , text);
#else
# define ELEKTRA_PRINT_DEBUG(text)
#endif

#if DEBUG && VERBOSE
# define ELEKTRA_PRINT_VERBOSE(text) printf("%s:%d: %s\n", __FILE__, __LINE__ , text);
#else
# define ELEKTRA_PRINT_VERBOSE(text)
#endif


#ifdef __cplusplus
namespace ckdb {
extern "C" {
#endif

typedef struct _Trie	Trie;
typedef struct _Split	Split;
typedef struct _Backend	Backend;

/* These define the type for pointers to all the kdb functions */
typedef int (*kdbOpenPtr)(Plugin *, Key *errorKey);
typedef int (*kdbClosePtr)(Plugin *, Key *errorKey);

typedef int (*kdbGetPtr)(Plugin *handle, KeySet *returned, Key *parentKey);
typedef int (*kdbSetPtr)(Plugin *handle, KeySet *returned, Key *parentKey);
typedef int (*kdbErrorPtr)(Plugin *handle, KeySet *returned, Key *parentKey);


typedef Backend* (*OpenMapper)(const char *,const char *,KeySet *);
typedef int (*CloseMapper)(Backend *);



/*****************
 * Key Flags
 *****************/

/**
 * Key Flags.
 *
 * Store a synchronizer state so that the Elektra knows if something
 * has changed or not.
 *
 * @ingroup backend
 */
typedef enum
{
	KEY_FLAG_SYNC=1,	/*!<
		Key need sync.
		If name, value or metadata
		are changed this flag will be set, so that the backend will sync
		the key to database.*/
	KEY_FLAG_RO_NAME=1<<1,	/*!<
		Read only flag for name.
		Key name is read only and not allowed
		to be changed. All attempts to change the name
		will lead to an error.
		Needed for meta keys and keys that are in a data
		structure that depends on name ordering.*/
	KEY_FLAG_RO_VALUE=1<<2,	/*!<
		Read only flag for value.
		Key value is read only and not allowed
		to be changed. All attempts to change the value
		will lead to an error.
		Needed for meta keys*/
	KEY_FLAG_RO_META=1<<3	/*!<
		Read only flag for meta.
		Key meta is read only and not allowed
		to be changed. All attempts to change the value
		will lead to an error.
		Needed for meta keys.*/
} keyflag_t;


/**
 * Ks Flags.
 *
 * Store a synchronizer state so that the Elektra knows if something
 * has changed or not.
 *
 * @ingroup backend
 */
typedef enum
{
	KS_FLAG_SYNC=1	/*!<
		KeySet need sync.
		If keys were popped from the Keyset
		this flag will be set, so that the backend will sync
		the keys to database.*/
} ksflag_t;


/**
 * The private Key struct.
 *
 * Its internal private attributes should not be accessed directly by regular
 * programs. Use the @ref key "Key access methods" instead.
 * Only a backend writer needs to have access to the private attributes of the
 * Key object which is defined as:
 * @code
typedef struct _Key Key;
 * @endcode
 *
 * @ingroup backend
 */
struct _Key
{
	/**
	 * The value, which is a NULL terminated string or binary.
	 * @see keyString(), keyBinary(),
	 * @see keyGetString(), keyGetBinary(),
	 * @see keySetString(), keySetBinary()
	 */
	union {char* c; void * v;} data;

	/**
	 * Size of the value, in bytes, including ending NULL.
	 * @see keyGetCommentSize(), keySetComment(), keyGetComment()
	 */
	size_t         dataSize;

	/**
	 * The name of the key.
	 * @see keySetName(), keySetName()
	 */
	char *         key;

	/**
	 * Size of the name, in bytes, including ending NULL.
	 * @see keyGetName(), keyGetNameSize(), keySetName()
	 */
	size_t         keySize;

	/**
	 * Size of the unescaped key name in bytes, including all NULL.
	 * @see keyBaseName(), keyUnescapedName()
	 */
	size_t         keyUSize;

	/**
	 * Some control and internal flags.
	 */
	keyflag_t      flags;

	/**
	 * In how many keysets the key resists.
	 * keySetName() is only allowed if ksReference is 0.
	 * @see ksPop(), ksAppendKey(), ksAppend()
	 */
	size_t        ksReference;

	/**
	 * All the key's meta information.
	 */
	KeySet *      meta;
};




/**
 * The private KeySet structure.
 *
 * Its internal private attributes should not be accessed directly by regular
 * programs. Use the @ref keyset "KeySet access methods" instead.
 * Only a backend writer needs to have access to the private attributes of the
 * KeySet object which is defined as:
 * @code
typedef struct _KeySet KeySet;
 * @endcode
 *
 * @ingroup backend
 */
struct _KeySet
{
	struct _Key **array;	/**<Array which holds the keys */

	size_t        size;	/**< Number of keys contained in the KeySet */
	size_t        alloc;	/**< Allocated size of array */

	struct _Key  *cursor;	/**< Internal cursor */
	size_t        current;	/**< Current position of cursor */

	/**
	 * Some control and internal flags.
	 */
	ksflag_t      flags;
};


/**
 * The access point to the key database.
 *
 * The structure which holds all information about loaded backends.
 *
 * Its internal private attributes should not be accessed directly.
 *
 * See kdb mount tool to mount new backends.
 *
 * KDB object is defined as:
 * @code
typedef struct _KDB KDB;
 * @endcode
 *
 * @see kdbOpen() and kdbClose() for external use
 * @ingroup backend
 */
struct _KDB {
	Trie *trie;		/*!< The pointer to the trie holding backends.*/

	Split *split;		/*!< A list of all mountpoints. It basically has the
				 same information than in the trie, but it is not trivial
				 to convert from one to the other.*/

	KeySet *modules;	/*!< A list of all modules loaded at the moment.*/

	Backend *defaultBackend;/*!< The default backend as fallback when nothing else is found.*/
};

/**
 * Holds all information related to a backend.
 *
 * Since Elektra 0.8 a Backend consists of many plugins.
 * A backend is responsible for everything related to the process
 * of writing out or reading in configuration.
 *
 * So this holds a list of set and get plugins.
 *
 * Backends are put together through the configuration
 * in system/elektra/mountpoints
 *
 * See kdb mount tool to mount new backends.
 *
 * To develop a backend you have first to develop plugins and describe
 * through dependencies how they belong together.
 *
 * @ingroup backend
 */
struct _Backend
{
	Key *mountpoint;	/*!< The mountpoint where the backend resides.
		The keyName() is the point where the backend was mounted.
		The keyValue() is the name of the backend without pre/postfix, e.g.
		filesys. */

	Plugin *setplugins[NR_OF_PLUGINS];
	Plugin *getplugins[NR_OF_PLUGINS];
	Plugin *errorplugins[NR_OF_PLUGINS];

	ssize_t usersize;	/*!< The size of the users key from the previous get.
		Needed to know if a key was removed from a keyset. */
	ssize_t systemsize;	/*!< The size of the systems key from the previous get.
		Needed to know if a key was removed from a keyset. */

	size_t refcounter;	/*!< This refcounter shows how often the backend
		is used.  Not cascading or default backends have 1 in it.
		More than two is not possible, because a backend
		can be only mounted in system and user each once.*/
};

/**
 * Holds all information related to a plugin.
 *
 * Since Elektra 0.8 a Backend consists of many plugins.
 *
 * A plugin should be reusable and only implement a single concern.
 * Plugins which are supplied with Elektra are located below src/plugins.
 * It is no problem that plugins are developed external too.
 *
 * @ingroup backend
 */
struct _Plugin
{
	KeySet *config;		/*!< This keyset contains configuration for the plugin.
		Direct below system/ there is the configuration supplied for the backend.
		Direct below user/ there is the configuration supplied just for the
		plugin, which should be of course prefered to the backend configuration.
		The keys inside contain information like /path which path should be used
		to write configuration to or /host to which host packets should be send.
		@see elektraPluginGetConfig() */

	kdbOpenPtr kdbOpen;	/*!< The pointer to kdbOpen_template() of the backend. */
	kdbClosePtr kdbClose;	/*!< The pointer to kdbClose_template() of the backend. */

	kdbGetPtr kdbGet;	/*!< The pointer to kdbGet_template() of the backend. */
	kdbSetPtr kdbSet;	/*!< The pointer to kdbSet_template() of the backend. */
	kdbErrorPtr kdbError;	/*!< The pointer to kdbError_template() of the backend. */

	const char *name;	/*!< The name of the module responsible for that plugin. */

	size_t refcounter;	/*!< This refcounter shows how often the plugin
		is used.  Not shared plugins have 1 in it */

	void *data;		/*!< This handle can be used for a plugin to store
		any data its want to. */
};


/** The private trie structure.
 *
 * A trie is a data structure which can handle the longest prefix matching very
 * fast. This is exactly what needs to be done when using kdbGet() and kdbSet()
 * in a hierarchy where backends are mounted - you need the backend mounted
 * closest to the parentKey.
 */
struct _Trie
{
	struct _Trie *children[KDB_MAX_UCHAR];/*!< The children building up the trie recursively */
	char *text[KDB_MAX_UCHAR];	/*!< Text identifying this node */
	size_t textlen[KDB_MAX_UCHAR];	/*!< Length of the text */
	Backend *value[KDB_MAX_UCHAR];	/*!< Pointer to a backend */
	Backend *empty_value;		/*!< Pointer to a backend for the empty string "" */
};


/** The private split structure.
 *
 * kdbSet() splits keysets. This structure contains arrays for
 * various information needed to process the keysets afterwards.
 */
struct _Split
{
	size_t size;		/*!< Number of keysets */
	size_t alloc;		/*!< How large the arrays are allocated  */
	KeySet **keysets;	/*!< The keysets */
	Backend **handles;	/*!< The KDB for the keyset */
	Key **parents;		/*!< The parentkey for the keyset.
				Is either the mountpoint of the backend
				or "user", "system" for the split root backends */
	int *syncbits;		/*!< Bits for various options:
				Bit 0: Is there any key in there which need to be synced?
				Bit 1: Do we need relative checks? (cascading backend?)*/
};

/***************************************
 *
 * Not exported functions, for internal use only
 *
 **************************************/

ssize_t keySetRaw(Key *key, const void *newBinary, size_t dataSize);

/*Methods for split keysets */
Split * elektraSplitNew(void);
void elektraSplitDel(Split *keysets);
void elektraSplitResize(Split *ret);
ssize_t elektraSplitAppend(Split *split, Backend *backend, Key *parentKey, int syncbits);
ssize_t elektraSplitSearchBackend(Split *split, Backend *backend, Key *key);
int elektraSplitSearchRoot(Split *split, Key *parentKey);
int elektraSplitBuildup (Split *split, KDB *handle, Key *parentKey);

/* for kdbGet() algorithm */
int elektraSplitAppoint (Split *split, KDB *handle, KeySet *ks);
int elektraSplitGet (Split *split, Key *warningKey, KDB *handle);
int elektraSplitMerge (Split *split, KeySet *dest);

/* for kdbSet() algorithm */
int elektraSplitDivide (Split *split, KDB *handle, KeySet *ks);
int elektraSplitSync (Split *split);
int elektraSplitPrepare (Split *split);
int elektraSplitUpdateSize (Split *split);


/*Backend handling*/
Backend* elektraBackendOpen(KeySet *elektra_config, KeySet *modules, Key *errorKey);
Backend* elektraBackendOpenMissing(Key *mountpoint);
Backend* elektraBackendOpenDefault(KeySet *modules, Key *errorKey);
Backend* elektraBackendOpenModules(KeySet *modules, Key *errorKey);
Backend* elektraBackendOpenVersion(Key *errorKey);
int elektraBackendClose(Backend *backend, Key *errorKey);

/*Plugin handling*/
int elektraProcessPlugin(Key *cur, int *pluginNumber, char **pluginName,
		char **referenceName, Key *errorKey);
int elektraProcessPlugins(Plugin **plugins, KeySet *modules, KeySet *referencePlugins,
		KeySet *config, KeySet *systemConfig, Key *errorKey);

Plugin* elektraPluginOpen(const char *backendname, KeySet *modules, KeySet *config, Key* errorKey);
int elektraPluginClose(Plugin *handle, Key *errorKey);
Plugin *elektraPluginMissing(void);
Plugin *elektraPluginVersion(void);

/*Trie handling*/
Trie *elektraTrieOpen(KeySet *config, KeySet *modules, Key *errorKey);
int elektraTrieClose (Trie *trie, Key *errorKey);
Backend* elektraTrieLookup(Trie *trie, const Key *key);
Trie* elektraTrieInsert(Trie *trie, const char *name, Backend *value);

/*Mounting handling */
int elektraMountOpen(KDB *kdb, KeySet *config, KeySet *modules, Key *errorKey);
int elektraMountDefault (KDB *kdb, KeySet *modules, Key *errorKey);
int elektraMountModules (KDB *kdb, KeySet *modules, Key *errorKey);
int elektraMountVersion (KDB *kdb, Key *errorKey);

int elektraMountBackend (KDB *kdb, Backend *backend, Key *errorKey);

Key* elektraMountGetMountpoint(KDB *handle, const Key *where);
Backend* elektraMountGetBackend(KDB *handle, const Key *key);

/*Private helper for keys*/
int keyInit(Key *key);
void keyVInit(Key *key, const char *keyname, va_list ap);

int keyClearSync (Key *key);

/*Private helper for keyset*/
int ksInit(KeySet *ks);
int ksClose(KeySet *ks);

ssize_t ksSearchInternal(const KeySet *ks, const Key *toAppend);

/*Used for internal memcpy/memmove*/
ssize_t elektraMemcpy (Key** array1, Key** array2, size_t size);
ssize_t elektraMemmove (Key** array1, Key** array2, size_t size);

char *elektraStrNDup (const char *s, size_t l);
ssize_t elektraFinalizeName(Key *key);
ssize_t elektraFinalizeEmptyName(Key *key);

char *elektraEscapeKeyNamePart(const char *source, char *dest);
size_t elektraUnescapeKeyName(const char *source, char *dest);

int elektraValidateKeyNamePart(const char *name);

/** Test a bit. @see set_bit(), clear_bit() */
#define test_bit(var,bit)            ((var) &   (bit))
/** Set a bit. @see clear_bit() */
#define set_bit(var,bit)             ((var) |=  (bit))
/** Clear a bit. @see set_bit() */
#define clear_bit(var,bit)           ((var) &= ~(bit))

#ifdef __cplusplus
}
}
#endif

#endif /* KDBPRIVATE_H */
