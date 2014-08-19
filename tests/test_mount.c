/*************************************************************************** 
 *           test_mount.c  - Test suite for mounting related issues
 *                  -------------------
 *  begin                : Sat Jul 10 2010
 *  copyright            : (C) 2010 by Markus Raab
 *  email                : elektra@markus-raab.org
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

#include <tests_internal.h>

KDB* kdb_new()
{
	KDB *kdb = elektraCalloc (sizeof (KDB));
	kdb->split = elektraSplitNew();
	return kdb;
}

Backend *b_new(const char *name, const char *value)
{
	Backend *backend = elektraCalloc (sizeof (Backend));
	backend->refcounter = 1;

	backend->mountpoint = keyNew (name, KEY_VALUE, value, KEY_END);
	keyIncRef (backend->mountpoint);

	return backend;
}

void kdb_del(KDB *kdb)
{
	elektraBackendClose (kdb->defaultBackend, 0);
	elektraTrieClose(kdb->trie, 0);
	elektraSplitDel (kdb->split);

	elektraFree (kdb);
}

void test_mount()
{
	printf ("test mount backend\n");

	KDB *kdb = kdb_new();
	elektraMountBackend (kdb, b_new("user", "user"), 0);
	succeed_if (kdb->trie, "there should be a trie");

	Key *mp = keyNew ("user", KEY_VALUE, "user", KEY_END);
	Key *sk = keyNew ("user", KEY_VALUE, "user", KEY_END);

	compare_key (elektraMountGetBackend (kdb, sk)->mountpoint, mp);
	compare_key (elektraMountGetMountpoint (kdb, sk), mp);

	keySetName (sk, "user/below");
	compare_key (elektraMountGetBackend (kdb, sk)->mountpoint, mp);
	compare_key (elektraMountGetMountpoint (kdb, sk), mp);

	keySetName (sk, "system");
	kdb->defaultBackend = b_new("", "default");
	succeed_if (elektraMountGetBackend (kdb, sk) == kdb->defaultBackend, "did not return default backend");

	keySetName (mp, "");
	keySetString (mp, "default");
	compare_key (elektraMountGetBackend (kdb, sk)->mountpoint, mp);
	compare_key (elektraMountGetMountpoint (kdb, sk), mp);

	keyDel (sk);
	keyDel (mp);

	kdb_del (kdb);
}

KeySet *modules_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/modules", KEY_END),
		KS_END);
}

KeySet *minimal_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		KS_END);
}


void test_minimaltrie()
{
	printf ("Test minimal mount\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, minimal_config(), modules, errorKey) == 0, "could not open minimal config")

	succeed_if(output_warnings (errorKey), "warnings found");
	succeed_if(output_error (errorKey), "error found");

	succeed_if (!kdb->trie, "minimal trie is null");

	keyDel (errorKey);
	ksDel (modules);
	kdb_del (kdb);
}

KeySet *simple_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/simple", KEY_END),
		KS_END);
}

void test_simple()
{
	printf ("Test simple mount\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, simple_config(), modules, errorKey) == 0, "could not open trie");

	succeed_if(output_warnings (errorKey), "warnings found");
	succeed_if(output_error (errorKey), "error found");

	exit_if_fail (kdb->trie, "kdb->trie was not build up successfully");

	Key *searchKey = keyNew("user", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");


	Key *mp = keyNew("user/tests/simple", KEY_VALUE, "simple", KEY_END);
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	compare_key(backend->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/below");
	Backend *b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);

	keyDel (errorKey);
	ksDel (modules);
	keyDel (mp);
	keyDel (searchKey);
	kdb_del (kdb);
}

KeySet *set_simple()
{
	return ksNew(50,
		keyNew("system/elektra/mountpoints/simple", KEY_END),

		keyNew("system/elektra/mountpoints/simple/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/anything", KEY_VALUE, "backend", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/simple/errorplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/errorplugins/#1default", KEY_VALUE, "default", KEY_END),

		keyNew("system/elektra/mountpoints/simple/getplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default", KEY_VALUE, "default", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/anything", KEY_VALUE, "plugin", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/backend/simple", KEY_END),

		keyNew("system/elektra/mountpoints/simple/setplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/setplugins/#1default", KEY_VALUE, "default", KEY_END),

		keyNew("system/elektra/mountpoints/simple/errorplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/errorplugins/#1default", KEY_VALUE, "default", KEY_END),
		KS_END);

}


KeySet *set_pluginconf()
{
	return ksNew( 10 ,
		keyNew ("system/anything", KEY_VALUE, "backend", KEY_END),
		keyNew ("system/more", KEY_END),
		keyNew ("system/more/config", KEY_END),
		keyNew ("system/more/config/below", KEY_END),
		keyNew ("system/path", KEY_END),
		keyNew ("user/anything", KEY_VALUE, "plugin", KEY_END),
		keyNew ("user/more", KEY_END),
		keyNew ("user/more/config", KEY_END),
		keyNew ("user/more/config/below", KEY_END),
		keyNew ("user/path", KEY_END),
		KS_END);
}

void test_simpletrie()
{
	printf ("Test simple mount with plugins\n");

	KDB *kdb = kdb_new();
	KeySet *modules = ksNew(0, KS_END);
	elektraModulesInit(modules, 0);

	KeySet *config = set_simple();
	ksAppendKey(config, keyNew("system/elektra/mountpoints", KEY_END));
	succeed_if (elektraMountOpen(kdb, config, modules, 0) == 0, "could not open mount");

	Key *key = keyNew("user/tests/backend/simple", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, key);

	keyAddBaseName(key, "somewhere"); keyAddBaseName(key, "deep"); keyAddBaseName(key, "below");
	Backend *backend2 = elektraTrieLookup(kdb->trie, key);
	succeed_if (backend == backend2, "should be same backend");

	succeed_if (backend->getplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->getplugins[1] != 0, "there should be a plugin");
	succeed_if (backend->getplugins[2] == 0, "there should be no plugin");

	succeed_if (backend->setplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->setplugins[1] != 0, "there should be a plugin");
	succeed_if (backend->setplugins[2] == 0, "there should be no plugin");

	Key *mp;
	succeed_if ((mp = backend->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "user/tests/backend/simple"), "wrong mountpoint for backend");
	succeed_if (!strcmp(keyString(mp), "simple"), "wrong name for backend");

	Plugin *plugin = backend->getplugins[1];

	KeySet *test_config = set_pluginconf();
	KeySet *cconfig = elektraPluginGetConfig (plugin);
	succeed_if (cconfig != 0, "there should be a config");
	compare_keyset(cconfig, test_config);
	ksDel (test_config);

	succeed_if (plugin->kdbGet != 0, "no get pointer");
	succeed_if (plugin->kdbSet != 0, "no set pointer");


	keyDel (key);
	elektraModulesClose (modules, 0);
	ksDel (modules);
	kdb_del (kdb);
}


KeySet *set_two()
{
	return ksNew(50,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),

		keyNew("system/elektra/mountpoints/simple/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/anything", KEY_VALUE, "backend", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/simple/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/simple/getplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default", KEY_VALUE, "default", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/anything", KEY_VALUE, "plugin", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/simple/getplugins/#1default/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/backend/simple", KEY_END),

		keyNew("system/elektra/mountpoints/simple/setplugins", KEY_END),
		keyNew("system/elektra/mountpoints/simple/setplugins/#1default", KEY_VALUE, "default", KEY_END),


		keyNew("system/elektra/mountpoints/two", KEY_END),

		keyNew("system/elektra/mountpoints/two/config", KEY_END),
		keyNew("system/elektra/mountpoints/two/config/anything", KEY_VALUE, "backend", KEY_END),
		keyNew("system/elektra/mountpoints/two/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/two/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/two/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/two/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/two/getplugins", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default", KEY_VALUE, "default", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config/anything", KEY_VALUE, "plugin", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config/more", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config/more/config", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config/more/config/below", KEY_END),
		keyNew("system/elektra/mountpoints/two/getplugins/#1default/config/path", KEY_END),

		keyNew("system/elektra/mountpoints/two/mountpoint", KEY_VALUE, "user/tests/backend/two", KEY_END),

		keyNew("system/elektra/mountpoints/two/setplugins", KEY_END),
		keyNew("system/elektra/mountpoints/two/setplugins/#1default", KEY_VALUE, "default", KEY_END),
		keyNew("system/elektra/mountpoints/two/setplugins/#2default", KEY_VALUE, "default", KEY_END),
		KS_END);
}

void test_two()
{
	printf ("Test two mounts\n");

	KDB *kdb = kdb_new();
	KeySet *modules = ksNew(0, KS_END);
	elektraModulesInit(modules, 0);

	KeySet *config = set_two();
	ksAppendKey(config, keyNew("system/elektra/mountpoints", KEY_END));
	succeed_if (elektraMountOpen (kdb, config, modules, 0) == 0, "could not open mount");

	Key *key = keyNew("user/tests/backend/simple", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, key);

	keyAddBaseName(key, "somewhere"); keyAddBaseName(key, "deep"); keyAddBaseName(key, "below");
	Backend *backend2 = elektraTrieLookup(kdb->trie, key);
	succeed_if (backend == backend2, "should be same backend");

	succeed_if (backend->getplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->getplugins[1] != 0, "there should be a plugin");
	succeed_if (backend->getplugins[2] == 0, "there should be no plugin");

	succeed_if (backend->setplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->setplugins[1] != 0, "there should be a plugin");
	succeed_if (backend->setplugins[2] == 0, "there should be no plugin");

	Key *mp;
	succeed_if ((mp = backend->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "user/tests/backend/simple"), "wrong mountpoint for backend");
	succeed_if (!strcmp(keyString(mp), "simple"), "wrong name for backend");

	Plugin *plugin = backend->getplugins[1];

	KeySet *test_config = set_pluginconf();
	KeySet *cconfig = elektraPluginGetConfig (plugin);
	succeed_if (cconfig != 0, "there should be a config");
	compare_keyset(cconfig, test_config);
	ksDel (test_config);

	succeed_if (plugin->kdbGet != 0, "no get pointer");
	succeed_if (plugin->kdbSet != 0, "no set pointer");

	keySetName(key, "user/tests/backend/two");
	Backend *two = elektraTrieLookup(kdb->trie, key);
	succeed_if (two != backend, "should be differnt backend");

	succeed_if ((mp = two->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "user/tests/backend/two"), "wrong mountpoint for backend two");
	succeed_if (!strcmp(keyString(mp), "two"), "wrong name for backend");

	keyDel (key);
	elektraModulesClose (modules, 0);
	ksDel (modules);
	kdb_del (kdb);
}


KeySet *set_us()
{
	return ksNew(50,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/user", KEY_END),
		keyNew("system/elektra/mountpoints/user/mountpoint", KEY_VALUE, "user", KEY_END),
		keyNew("system/elektra/mountpoints/system", KEY_END),
		keyNew("system/elektra/mountpoints/system/mountpoint", KEY_VALUE, "system", KEY_END),
		KS_END);

}

void test_us()
{
	printf ("Test mounting of user and system backends\n");

	KDB *kdb = kdb_new();
	KeySet *modules = ksNew(0, KS_END);
	elektraModulesInit(modules, 0);

	KeySet *config = set_us();
	ksAppendKey(config, keyNew("system/elektra/mountpoints", KEY_END));
	succeed_if (elektraMountOpen(kdb, config, modules, 0) == 0, "could not open mount");

	Key *key = keyNew("user/anywhere/backend/simple", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, key);

	keyAddBaseName(key, "somewhere"); keyAddBaseName(key, "deep"); keyAddBaseName(key, "below");
	Backend *backend2 = elektraTrieLookup(kdb->trie, key);
	succeed_if (backend == backend2, "should be same backend");

	succeed_if (backend->getplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->getplugins[1] == 0, "there should be no plugin");
	succeed_if (backend->getplugins[2] == 0, "there should be no plugin");

	succeed_if (backend->setplugins[0] == 0, "there should be no plugin");
	exit_if_fail (backend->setplugins[1] == 0, "there should be no plugin");
	succeed_if (backend->setplugins[2] == 0, "there should be no plugin");

	Key *mp;
	succeed_if ((mp = backend->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "user"), "wrong mountpoint for backend");
	succeed_if (!strcmp(keyString(mp), "user"), "wrong name for backend");


	keySetName(key, "system/anywhere/tests/backend/two");
	Backend *two = elektraTrieLookup(kdb->trie, key);
	succeed_if (two != backend, "should be differnt backend");

	succeed_if ((mp = two->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "system"), "wrong mountpoint for backend two");
	succeed_if (!strcmp(keyString(mp), "system"), "wrong name for backend");

	keyDel (key);
	elektraModulesClose (modules, 0);
	ksDel (modules);
	kdb_del (kdb);

}

KeySet *endings_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/slash", KEY_END),
		keyNew("system/elektra/mountpoints/slash/mountpoint", KEY_VALUE, "user/endings", KEY_END),
		keyNew("system/elektra/mountpoints/hash", KEY_END),
		keyNew("system/elektra/mountpoints/hash/mountpoint", KEY_VALUE, "user/endings#", KEY_END),
		keyNew("system/elektra/mountpoints/space", KEY_END),
		keyNew("system/elektra/mountpoints/space/mountpoint", KEY_VALUE, "user/endings ", KEY_END),
		keyNew("system/elektra/mountpoints/endings", KEY_END),
		keyNew("system/elektra/mountpoints/endings/mountpoint", KEY_VALUE, "user/endings\200", KEY_END),
		KS_END);
}

void test_endings()
{
	printf ("Test mounting with different endings\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, endings_config(), modules, errorKey) == 0, "could not open mount");

	succeed_if(output_warnings (errorKey), "warnings found");
	succeed_if(output_error (errorKey), "error found");

	exit_if_fail (kdb->trie, "kdb->trie was not build up successfully");

	Key *searchKey = keyNew("user", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");


	Key *mp = keyNew("user/endings", KEY_VALUE, "slash", KEY_END);
	keySetName(searchKey, "user/endings");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	compare_key(backend->mountpoint, mp);


	keySetName(searchKey, "user/endings#");
	keySetName(mp, "user/endings#");
	keySetString(mp, "hash");
	Backend *b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend != b2, "should be other backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "user/endings/_");
	keySetName(mp, "user/endings");
	keySetString(mp, "slash");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be the same backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "user/endings/X");
	keySetName(mp, "user/endings");
	keySetString(mp, "slash");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be the same backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "user/endings_");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!b2, "there should be no backend");


	keySetName(searchKey, "user/endingsX");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!b2, "there should be no backend");


	keySetName(searchKey, "user/endings!");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!b2, "there should be no backend");


	keySetName(searchKey, "user/endings ");
	keySetName(mp, "user/endings ");
	keySetString(mp, "space");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend != b2, "should be other backend");
	compare_key(b2->mountpoint, mp);

	keySetName(searchKey, "user/endings\200");
	keySetName(mp, "user/endings\200");
	keySetString(mp, "endings");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend != b2, "should be other backend");
	compare_key(b2->mountpoint, mp);

	// output_trie(trie);

	keyDel (errorKey);
	ksDel (modules);
	keyDel (mp);
	keyDel (searchKey);
	kdb_del (kdb);
}

KeySet *oldroot_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/root", KEY_END),
		keyNew("system/elektra/mountpoints/root/mountpoint", KEY_VALUE, "", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/simple", KEY_END),
		KS_END);
}

void test_oldroot()
{
	printf ("Test mounting with old root\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, oldroot_config(), modules, errorKey) == 0, "root should be mounted as default");

	succeed_if(output_warnings (errorKey), "warnings found");
	succeed_if(output_error (errorKey), "error found");

	exit_if_fail (kdb->trie, "trie was not build up successfully");

	Key *searchKey = keyNew("user", KEY_END);
	Key *rmp = keyNew("", KEY_VALUE, "root", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no root backend");


	Key *mp = keyNew("user/tests/simple", KEY_VALUE, "simple", KEY_END);
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	compare_key(backend->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/below");
	Backend *b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);

	keyDel (mp);
	keyDel (rmp);

	keyDel (searchKey);

	kdb_del (kdb);
	keyDel (errorKey);
	ksDel (modules);
}

KeySet *cascading_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "/tests/simple", KEY_END),
		KS_END);
}

void keySetCascading(Key *key, const char* name)
{
	size_t size = elektraStrLen(name) + 1;
	if (size < 2) return;

	elektraFree (key->key);
	key->key = elektraMalloc (size);
	key->keySize = size;
	key->key[0] = '/';
	strcpy (key->key+1, name);
}

void test_cascading()
{
	printf ("Test simple mount with cascading\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, cascading_config(), modules, errorKey) == 0, "could not open trie");

	succeed_if(output_warnings (errorKey), "warnings found");
	succeed_if(output_error (errorKey), "error found");

	exit_if_fail (kdb->trie, "kdb->trie was not build up successfully");

	// output_trie (kdb->trie);

	Key *searchKey = keyNew("user", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");

	keySetName(searchKey, "system");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");


	Key *mp = keyNew("", KEY_VALUE, "simple", KEY_END);
	keySetCascading (mp, "tests/simple");

	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	compare_key(backend->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/below");
	Backend *b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "system/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	compare_key(backend->mountpoint, mp);

	keySetName(searchKey, "system/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "system/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);


	keyDel (errorKey);
	ksDel (modules);
	keyDel (mp);
	keyDel (searchKey);
	kdb_del (kdb);
}


KeySet *root_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/root", KEY_END),
		keyNew("system/elektra/mountpoints/root/mountpoint", KEY_VALUE, "/", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/simple", KEY_END),
		KS_END);
}

void test_root()
{
	printf ("Test mounting with root\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, root_config(), modules, errorKey) == 0, "could not buildup mount");

	succeed_if(output_warnings (errorKey), "warnings found");
	succeed_if(output_error (errorKey), "error found");

	exit_if_fail (kdb->trie, "trie was not build up successfully");

	Key *searchKey = keyNew("", KEY_END);
	Key *rmp = keyNew("", KEY_VALUE, "root", KEY_END);
	keySetCascading (rmp, "");
	Backend *b2 = 0;

	keySetName (searchKey, "user");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	compare_key(b2->mountpoint, rmp);


	Backend *backend = 0;
	Key *mp = keyNew("user/tests/simple", KEY_VALUE, "simple", KEY_END);
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	compare_key(backend->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);

	keyDel (mp);
	keyDel (rmp);

	keyDel (searchKey);

	kdb_del (kdb);
	keyDel (errorKey);
	ksDel (modules);
}

void test_default()
{
	printf ("Test mounting with default\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, root_config(), modules, errorKey) == 0, "could not buildup mount");
	succeed_if (elektraMountDefault(kdb, modules, errorKey) == 0, "could not mount default backend");

	succeed_if(output_warnings (errorKey), "warnings found");
	succeed_if(output_error (errorKey), "error found");

	exit_if_fail (kdb->trie, "trie was not build up successfully");

	// output_trie (kdb->trie);

	Key *searchKey = keyNew("", KEY_END);
	Key *rmp = keyNew("", KEY_VALUE, "root", KEY_END);
	keySetCascading (rmp, "");
	Backend *b2 = 0;

	keySetName (searchKey, "user");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	compare_key(b2->mountpoint, rmp);


	Backend *backend = 0;
	Key *mp = keyNew("user/tests/simple", KEY_VALUE, "simple", KEY_END);
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	compare_key(backend->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);

	Key *dmp = keyNew ("", KEY_VALUE, "default", KEY_END);
	keySetName(searchKey, "system/elektra");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (b2 == kdb->defaultBackend, "should be the default backend");
	compare_key(b2->mountpoint, dmp);

	keySetName(searchKey, "system/elektra/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (b2 == kdb->defaultBackend, "should be the default backend");
	compare_key(b2->mountpoint, dmp);

	keyDel (dmp);
	keyDel (mp);
	keyDel (rmp);

	keyDel (searchKey);

	kdb_del (kdb);
	keyDel (errorKey);
	ksDel (modules);
}

void test_modules()
{
	printf ("Test mounting with modules\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, root_config(), modules, errorKey) == 0, "could not buildup mount");
	succeed_if (elektraMountDefault(kdb, modules, errorKey) == 0, "could not mount default backend");
	succeed_if (elektraMountModules(kdb, modules, errorKey) == 0, "could not mount modules");

	succeed_if(output_warnings (errorKey), "warnings found");
	succeed_if(output_error (errorKey), "error found");

	exit_if_fail (kdb->trie, "trie was not build up successfully");

	// output_trie (kdb->trie);

	Key *searchKey = keyNew("", KEY_END);
	Key *rmp = keyNew("", KEY_VALUE, "root", KEY_END);
	keySetCascading (rmp, "");
	Backend *b2 = 0;

	keySetName (searchKey, "user");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	compare_key(b2->mountpoint, rmp);


	Backend *backend = 0;
	Key *mp = keyNew("user/tests/simple", KEY_VALUE, "simple", KEY_END);
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	compare_key(backend->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	compare_key(b2->mountpoint, mp);

	Key *dmp = keyNew ("", KEY_VALUE, "default", KEY_END);
	keySetName(searchKey, "system/elektra");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (b2 == kdb->defaultBackend, "should be the default backend");
	compare_key(b2->mountpoint, dmp);

	keySetName(searchKey, "system/elektra/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (b2 == kdb->defaultBackend, "should be the default backend");
	compare_key(b2->mountpoint, dmp);

	Key *mmp = keyNew ("system/elektra/modules", KEY_VALUE, "modules", KEY_END);
	keyAddBaseName (mmp, "default");

	/*
	keySetName(searchKey, "system/elektra/modules/default");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (b2 != kdb->defaultBackend, "should not be the default backend");
	compare_key(b2->mountpoint, mmp);
	*/

	keyDel (mmp);
	keyDel (dmp);
	keyDel (mp);
	keyDel (rmp);

	keyDel (searchKey);

	kdb_del (kdb);
	keyDel (errorKey);
	ksDel (modules);
}

void test_kdbopen()
{
	printf ("Test mounting modules\n");

	Key *errorKey = keyNew("", KEY_END);
	KDB *kdb = kdbOpen (errorKey);

	output_trie (kdb->trie);

	kdbClose (kdb, errorKey);

	succeed_if(output_warnings (errorKey), "warnings found");
	succeed_if(output_error (errorKey), "error found");

	keyDel (errorKey);
}

int main(int argc, char** argv)
{
	printf("TRIE       TESTS\n");
	printf("==================\n\n");

	init (argc, argv);

	test_mount();
	test_minimaltrie();
	test_simple();
	test_simpletrie();
	test_two();
	test_us();
	test_endings();
	test_oldroot();
	test_cascading();
	test_root();
	test_default();
	test_modules();
	// test_kdbopen();

	printf("\ntest_trie RESULTS: %d test(s) done. %d error(s).\n", nbTest, nbError);

	return nbError;
}

