/*************************************************************************** 
 *           test_splitset.c  - Test suite for split keyset data structure
 *                  -------------------
 *  begin                : Tue Jun 29 2010
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


KeySet *set_three()
{
	return ksNew(50,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/system", KEY_END),
		keyNew("system/elektra/mountpoints/system/mountpoint", KEY_VALUE, "system", KEY_END),
		keyNew("system/elektra/mountpoints/userin", KEY_END),
		keyNew("system/elektra/mountpoints/userin/mountpoint", KEY_VALUE, "user/invalid", KEY_END),
		keyNew("system/elektra/mountpoints/userva", KEY_END),
		keyNew("system/elektra/mountpoints/userva/mountpoint", KEY_VALUE, "user/valid", KEY_END),
		KS_END);
}


KeySet *set_realworld()
{
	return ksNew(50,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/root", KEY_END),
		keyNew("system/elektra/mountpoints/root/mountpoint", KEY_VALUE, "/", KEY_END),
		keyNew("system/elektra/mountpoints/default", KEY_END),
		keyNew("system/elektra/mountpoints/default/mountpoint", KEY_VALUE, "system/elektra", KEY_END),
		keyNew("system/elektra/mountpoints/users", KEY_END),
		keyNew("system/elektra/mountpoints/users/mountpoint", KEY_VALUE, "system/users", KEY_END),
		keyNew("system/elektra/mountpoints/groups", KEY_END),
		keyNew("system/elektra/mountpoints/groups/mountpoint", KEY_VALUE, "system/groups", KEY_END),
		keyNew("system/elektra/mountpoints/hosts", KEY_END),
		keyNew("system/elektra/mountpoints/hosts/mountpoint", KEY_VALUE, "system/hosts", KEY_END),
		keyNew("system/elektra/mountpoints/kde", KEY_END),
		keyNew("system/elektra/mountpoints/kde/mountpoint", KEY_VALUE, "user/sw/kde/default", KEY_END),
		keyNew("system/elektra/mountpoints/app1", KEY_END),
		keyNew("system/elektra/mountpoints/app1/mountpoint", KEY_VALUE, "user/sw/apps/app1/default", KEY_END),
		keyNew("system/elektra/mountpoints/app2", KEY_END),
		keyNew("system/elektra/mountpoints/app2/mountpoint", KEY_VALUE, "user/sw/apps/app2", KEY_END),
		KS_END);
}

KDB* kdb_open()
{
	KDB *handle = elektraCalloc(sizeof(struct _KDB));
	handle->split = elektraSplitNew();
	handle->modules = ksNew(0, KS_END);
	elektraModulesInit(handle->modules, 0);
	return handle;
}

void kdb_close(KDB *kdb)
{
	kdbClose (kdb, 0);
}


void test_needsync()
{
	printf ("Test needs sync\n");

	KDB *handle = kdb_open();
	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not mount default backends");

	KeySet *ks = ksNew (5,
			keyNew("user/abc", KEY_END),
			KS_END);
	Split *split = elektraSplitNew();

	Key *parent = keyNew("user", KEY_VALUE, "parent", KEY_END);

	succeed_if (split->size == 0, "size should be zero");
	succeed_if (split->alloc == APPROXIMATE_NR_OF_BACKENDS, "initial size not correct");

	succeed_if (elektraSplitBuildup(split, handle, parent) == 1, "buildup failure");
	succeed_if (elektraSplitDivide(split, handle, ks) == 1, "there should be a need sync");

	succeed_if (split->handles[0] == handle->defaultBackend, "handle not correct");
	compare_keyset(split->keysets[0], ks);
	succeed_if (split->syncbits[0] & 1, "sync bit should be set");

	succeed_if (split->size == 1, "size should be one");
	succeed_if (split->alloc == APPROXIMATE_NR_OF_BACKENDS, "should stay same");

	elektraSplitDel (split);


	split = elektraSplitNew();

	succeed_if (split->size == 0, "size should be zero");
	succeed_if (split->alloc == APPROXIMATE_NR_OF_BACKENDS, "initial size not correct");

	clear_sync (ks);
	succeed_if (elektraSplitBuildup(split, handle, parent) == 1, "buildup failure");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "there should not be a need sync");
	succeed_if (split->handles[0] == handle->defaultBackend, "handle not correct");
	compare_keyset(split->keysets[0], ks);
	succeed_if ((split->syncbits[0] & 1) == 0, "sync bit should be set");

	succeed_if (split->size == 1, "size should be one");
	succeed_if (split->alloc == APPROXIMATE_NR_OF_BACKENDS, "should stay same");

	elektraSplitDel (split);


	split = elektraSplitNew();

	ksAppendKey(ks, keyNew("user/key1", KEY_END));
	ksAppendKey(ks, keyNew("user/key2", KEY_END));
	ksAppendKey(ks, keyNew("user/key3", KEY_END));
	ksAppendKey(ks, keyNew("user/key4", KEY_END));
	ksAppendKey(ks, keyNew("user/key5", KEY_END));

	succeed_if (elektraSplitBuildup(split, handle, parent) == 1, "buildup failure");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "there should be a need sync");
	succeed_if (split->handles[0] == handle->defaultBackend, "handle not correct");
	compare_keyset(split->keysets[0], ks);
	succeed_if (split->syncbits[0] & 1, "sync bit should be set");
	elektraSplitDel (split);


	keyDel (parent);
	ksDel (ks);
	kdb_close(handle);
}



void test_mount()
{
	printf ("Test mount split\n");

	KDB *handle = kdb_open();

	succeed_if (elektraMountOpen (handle, set_us(), handle->modules, 0) == 0, "could not open mountpoints");
	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not open default backend");

	KeySet *ks = ksNew (
		5,
		keyNew ("user/valid/key1", KEY_END),
		keyNew ("user/valid/key2", KEY_END),
		keyNew ("system/valid/key1", KEY_END),
		keyNew ("system/valid/key2", KEY_END),
		KS_END);
	KeySet *split1 = ksNew (
		3,
		keyNew ("user/valid/key1", KEY_END),
		keyNew ("user/valid/key2", KEY_END),
		KS_END);
	KeySet *split2 = ksNew (
		3,
		keyNew ("system/valid/key1", KEY_END),
		keyNew ("system/valid/key2", KEY_END),
		KS_END);


	Split *split = elektraSplitNew();
	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");
	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->syncbits[0] == 1, "system part need to by synced");
	succeed_if (split->syncbits[1] == 1, "user part need to by synced");
	succeed_if (split->size == 3, "not split according user, system");
	succeed_if (ksGetSize(split->keysets[0]) == 2, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 2, "size of keyset not correct");
	compare_keyset(split->keysets[1], split1);
	compare_keyset(split->keysets[0], split2);
	elektraSplitDel (split);


	split = elektraSplitNew();
	clear_sync (ks);
	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should not need sync");
	succeed_if (split->alloc == APPROXIMATE_NR_OF_BACKENDS, "should stay same");
	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->syncbits[0] == 0, "system part does not need to by synced");
	succeed_if (split->syncbits[1] == 0, "user part does not need to by synced");
	succeed_if (ksGetSize(split->keysets[0]) == 2, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 2, "size of keyset not correct");
	succeed_if (split->size == 3, "not split according user, system");
	elektraSplitDel (split);

	split = elektraSplitNew();
	keySetString(ksLookupByName(ks, "user/valid/key2", 0), "value");
	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");
	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->syncbits[0] == 0, "system part does not need to by synced");
	succeed_if (split->syncbits[1] == 1, "user part need to by synced");
	succeed_if (ksGetSize(split->keysets[0]) == 2, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 2, "size of keyset not correct");
	succeed_if (split->size == 3, "not split according user, system");
	elektraSplitDel (split);

	split = elektraSplitNew();
	keySetString(ksLookupByName(ks, "system/valid/key2", 0), "value");
	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");
	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->syncbits[0] == 1, "system part need to by synced");
	succeed_if (split->syncbits[1] == 1, "user part need to by synced");
	succeed_if (ksGetSize(split->keysets[0]) == 2, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 2, "size of keyset not correct");
	succeed_if (split->size == 3, "not split according user, system");
	elektraSplitDel (split);


	ksDel (ks);
	ksDel (split1);
	ksDel (split2);
	kdb_close(handle);
}

void test_easyparent()
{
	printf ("Test parent separation of user and system (default Backend)\n");

	KDB *handle = kdb_open();
	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not open default backend");
	KeySet *ks = ksNew (
		8,
		keyNew ("user/valid", KEY_END),
		keyNew ("user/valid/key1", KEY_END),
		keyNew ("user/valid/key2", KEY_END),
		keyNew ("system/valid", KEY_END),
		keyNew ("system/valid/key1", KEY_END),
		keyNew ("system/valid/key2", KEY_END),
		KS_END);
	KeySet *split1 = ksNew ( 5,
		keyNew ("system/valid", KEY_END),
		keyNew ("system/valid/key1", KEY_END),
		keyNew ("system/valid/key2", KEY_END),
		KS_END);
	KeySet *split2 = ksNew ( 5,
		keyNew ("user/valid", KEY_END),
		keyNew ("user/valid/key1", KEY_END),
		keyNew ("user/valid/key2", KEY_END),
		KS_END);
	Key *parentKey;
	Split *split;


	parentKey = keyNew ("user", KEY_END);
	split = elektraSplitNew();
	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");


	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 2, "not split according user, system");
	succeed_if (split->syncbits[0] & 1, "system part need to by synced");
	succeed_if (split->syncbits[1] & 1, "user part need to be synced");
	compare_keyset(split->keysets[0], split1);
	compare_keyset(split->keysets[1], split2);

	elektraSplitDel (split);
	keyDel (parentKey);

	parentKey = keyNew ("system", KEY_END);
	split = elektraSplitNew();
	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");


	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 2, "not split according user, system");
	succeed_if (split->syncbits[0] & 1, "system part need to by synced");
	succeed_if (split->syncbits[1] & 1, "user part need to be synced");
	compare_keyset(split->keysets[0], split1);
	compare_keyset(split->keysets[1], split2);

	elektraSplitDel (split);
	keyDel (parentKey);

	ksDel (ks);
	ksDel (split1);
	ksDel (split2);
	kdb_close(handle);
}

void test_optimize()
{
	printf ("Test optimization split (user, system in trie)\n");

	KDB *handle = kdb_open();

	succeed_if (elektraMountOpen (handle, set_us(), handle->modules, 0) == 0, "could not open mountpoints");
	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not open default backend");

	KeySet *ks = ksNew ( 5,
		keyNew ("system/valid/key1", KEY_END),
		keyNew ("system/valid/key2", KEY_END),
		keyNew ("user/valid/key1", KEY_END),
		keyNew ("user/valid/key2", KEY_END),
		KS_END);
	KeySet *split1 = ksNew ( 3,
		keyNew ("system/valid/key1", KEY_END),
		keyNew ("system/valid/key2", KEY_END),
		KS_END);
	KeySet *split2 = ksNew ( 3,
		keyNew ("user/valid/key1", KEY_END),
		keyNew ("user/valid/key2", KEY_END),
		KS_END);
	Split *split = elektraSplitNew();
	Key *key;


	ksRewind (ks);
	while ((key = ksNext(ks)) != 0)
	{
		if (keyIsUser(key) == 1) keyClearSync(key);
	}

	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 3, "not split according user, system");
	succeed_if (split->syncbits[0] == 1, "system part not optimized");
	succeed_if (split->syncbits[1] == 0, "user part need to by synced");
	compare_keyset(split->keysets[0], split1);
	compare_keyset(split->keysets[1], split2);

	elektraSplitDel (split);


	split = elektraSplitNew();
	clear_sync (ks);

	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 3, "not split according user, system");
	succeed_if (split->syncbits[0] == 0, "system part not optimized");
	succeed_if (split->syncbits[1] == 0, "user part not optimized");
	compare_keyset(split->keysets[0], split1);
	compare_keyset(split->keysets[1], split2);

	elektraSplitDel (split);



	ksRewind (ks);
	while ((key = ksNext(ks)) != 0)
	{
		key->flags = KEY_FLAG_SYNC;
	}


	split = elektraSplitNew();
	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 3, "not split according user, system");
	succeed_if (split->syncbits[0] == 1, "optimized too much");
	succeed_if (split->syncbits[1] == 1, "optimized too much");
	compare_keyset(split->keysets[0], split1);
	compare_keyset(split->keysets[1], split2);

	elektraSplitDel (split);


	ksDel (ks);
	ksDel (split1);
	ksDel (split2);

	kdb_close(handle);
}

void test_three()
{
	printf ("Test three mountpoints\n");

	KDB *handle = kdb_open();

	succeed_if (elektraMountOpen (handle, set_three(), handle->modules, 0) == 0, "could not open mountpoints");
	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not open default backend");

	KeySet *ks = ksNew (
		18,
		keyNew ("system/valid", KEY_END),
		keyNew ("system/valid/key1", KEY_END),
		keyNew ("system/valid/key2", KEY_END),
		keyNew ("system/valid/key3", KEY_END),
		keyNew ("user/invalid", KEY_END),
		keyNew ("user/invalid/key1", KEY_END),
		keyNew ("user/invalid/key2", KEY_END),
		keyNew ("user/valid", KEY_END),
		keyNew ("user/valid/key1", KEY_END),
		keyNew ("user/outside", KEY_END),
		KS_END);
	KeySet *split0 = ksNew (
		9,
		keyNew ("system/valid", KEY_END),
		keyNew ("system/valid/key1", KEY_END),
		keyNew ("system/valid/key2", KEY_END),
		keyNew ("system/valid/key3", KEY_END),
		KS_END);
	KeySet *split1 = ksNew (
		9,
		keyNew ("user/invalid", KEY_END),
		keyNew ("user/invalid/key1", KEY_END),
		keyNew ("user/invalid/key2", KEY_END),
		KS_END);
	KeySet *split2 = ksNew (
		9,
		keyNew ("user/valid", KEY_END),
		keyNew ("user/valid/key1", KEY_END),
		KS_END);
	KeySet *split3 = ksNew (
		9,
		keyNew ("user/outside", KEY_END),
		KS_END);


	Split *split = elektraSplitNew();
	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 5, "not split according three");
	succeed_if (split->syncbits[0] == 1, "system part need to by synced");
	succeed_if (split->syncbits[1] == 1, "user part need to by synced");
	succeed_if (split->syncbits[2] == 1, "user part need to by synced");
	succeed_if (split->syncbits[3] == 2, "system/elektra default part need to by synced");
	succeed_if (split->syncbits[4] == 3, "user default part need to by synced");
	succeed_if (ksGetSize(split->keysets[0]) == 4, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 3, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[2]) == 2, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[3]) == 0, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[4]) == 1, "size of keyset not correct");
	compare_keyset(split->keysets[0], split0);
	compare_keyset(split->keysets[1], split1);
	compare_keyset(split->keysets[2], split2);
	compare_keyset(split->keysets[4], split3);

	elektraSplitPrepare(split);

	/* Prepare should not change anything here (everything needs sync) */
	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 4, "not split according three");
	succeed_if (split->syncbits[0] == 1, "system part need to by synced");
	succeed_if (split->syncbits[1] == 1, "user part need to by synced");
	succeed_if (split->syncbits[2] == 1, "user part need to by synced");
	succeed_if (split->syncbits[3] == 3, "user root part need to by synced");
	succeed_if (ksGetSize(split->keysets[0]) == 4, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 3, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[2]) == 2, "size of keyset not correct");
	succeed_if (ksGetSize(split->keysets[3]) == 1, "size of keyset not correct");
	compare_keyset(split->keysets[0], split0);
	compare_keyset(split->keysets[1], split1);
	compare_keyset(split->keysets[2], split2);
	compare_keyset(split->keysets[3], split3);

	elektraSplitDel (split);



	ksDel (ks);
	ksDel (split0);
	ksDel (split1);
	ksDel (split2);
	ksDel (split3);
	kdb_close(handle);
}


void test_userremove()
{
	printf ("Test user removing\n");
	Key *parent = 0;
	KDB *handle = kdb_open();

	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not open default backend");
	handle->defaultBackend->usersize = 2;
	/* So we had 2 keys before in the keyset */

	KeySet *ks = ksNew ( 3,
		keyNew ("user/valid/key", KEY_END),
		KS_END);



	Split *split = elektraSplitNew();

	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");
	succeed_if (elektraSplitSync (split) == 1, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 2, "everything is in two keyset");
	succeed_if (ksGetSize(split->keysets[1]) == 1, "wrong size");
	compare_keyset(split->keysets[1], ks);

	elektraSplitDel (split);


	split = elektraSplitNew();

	parent = keyNew ("user/valid", KEY_END);
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");
	succeed_if (elektraSplitSync (split) == 1, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 1, "everything is in two keyset");
	// output_split(split);
	succeed_if (ksGetSize(split->keysets[0]) == 1, "wrong size");
	compare_keyset(split->keysets[0], ks);
	keyDel (parent);

	elektraSplitDel (split);


	split = elektraSplitNew();

	parent = keyNew ("system/valid", KEY_END);
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should need sync");
	succeed_if (elektraSplitSync (split) == 0, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 1, "everything is in two keyset");
	// output_split(split);
	succeed_if (ksGetSize(split->keysets[0]) == 0, "should be dropped");
	keyDel (parent);

	elektraSplitDel (split);


	/* But it should even need sync when we don't have any unsynced keys! */
	clear_sync(ks);
	split = elektraSplitNew();

	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "no key inside needs sync");
	succeed_if (elektraSplitSync (split) == 1, "but we need sync because of the size mismatch");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 2, "everything is in two keyset");
	succeed_if (ksGetSize(split->keysets[1]) == 1, "wrong size");
	compare_keyset (split->keysets[1], ks);

	elektraSplitDel (split);


	split = elektraSplitNew();

	parent = keyNew ("user/valid", KEY_END);
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should need sync");
	succeed_if (elektraSplitSync (split) == 1, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 1, "everything is in two keyset");
	// output_split(split);
	succeed_if (ksGetSize(split->keysets[0]) == 1, "wrong size");
	compare_keyset(split->keysets[0], ks);
	keyDel (parent);

	elektraSplitDel (split);


	split = elektraSplitNew();

	parent = keyNew ("system/valid", KEY_END);
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should not need sync");
	succeed_if (elektraSplitSync (split) == 0, "should not need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 1, "everything is in two keyset");
	// output_split(split);
	succeed_if (ksGetSize(split->keysets[0]) == 0, "should be dropped");
	keyDel (parent);

	elektraSplitPrepare(split);
	succeed_if (split->size == 0, "no remaining keyset");

	elektraSplitDel (split);



	ksDel (ks);
	kdb_close(handle);
}


void test_systemremove()
{
	printf ("Test system removing\n");
	Key *parent = 0;
	KDB *handle = kdb_open();

	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not open default backend");
	handle->defaultBackend->systemsize = 2;
	/* So we had 2 keys before in the keyset */

	KeySet *ks = ksNew ( 3,
		keyNew ("system/valid/key", KEY_END),
		KS_END);



	Split *split = elektraSplitNew();

	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");
	succeed_if (elektraSplitSync (split) == 1, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 2, "everything is in two keyset");
	succeed_if (ksGetSize(split->keysets[0]) == 1, "wrong size");
	compare_keyset(split->keysets[0], ks);

	elektraSplitDel (split);


	split = elektraSplitNew();

	parent = keyNew ("system/valid", KEY_END);
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");
	succeed_if (elektraSplitSync (split) == 1, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 1, "everything is in one keyset");
	// output_split(split);
	succeed_if (ksGetSize(split->keysets[0]) == 1, "wrong size");
	compare_keyset(split->keysets[0], ks);
	keyDel (parent);

	elektraSplitDel (split);


	split = elektraSplitNew();

	parent = keyNew ("user/valid", KEY_END);
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should not need sync");
	succeed_if (elektraSplitSync (split) == 0, "should not need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 1, "everything is in one keyset");
	// output_split(split);
	succeed_if (ksGetSize(split->keysets[0]) == 0, "should be dropped");
	keyDel (parent);

	elektraSplitDel (split);


	/* But it should even need sync when we don't have any unsynced keys! */
	clear_sync(ks);
	split = elektraSplitNew();

	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "no key inside needs sync");
	succeed_if (elektraSplitSync (split) == 1, "but we need sync because of the size mismatch");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 2, "everything is in two keyset");
	succeed_if (ksGetSize(split->keysets[0]) == 1, "wrong size");
	compare_keyset(split->keysets[0], ks);

	elektraSplitDel (split);


	split = elektraSplitNew();

	parent = keyNew ("system/valid", KEY_END);
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should not need sync");
	succeed_if (elektraSplitSync (split) == 1, "should need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 1, "everything is in two keyset");
	// output_split(split);
	succeed_if (ksGetSize(split->keysets[0]) == 1, "wrong size");
	compare_keyset(split->keysets[0], ks);
	keyDel (parent);

	elektraSplitDel (split);


	split = elektraSplitNew();

	parent = keyNew ("user/valid", KEY_END);
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should not need sync");
	succeed_if (elektraSplitSync (split) == 0, "should not need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 1, "everything is in two keyset");
	// output_split(split);
	succeed_if (ksGetSize(split->keysets[0]) == 0, "should be dropped");
	keyDel (parent);

	elektraSplitPrepare(split);
	succeed_if (split->size == 0, "no remaining keyset");

	elektraSplitDel (split);



	ksDel (ks);
	kdb_close(handle);
}


void test_emptyremove()
{
	printf ("Test empty removing\n");

	KDB *handle = kdb_open();

	Key *parent = 0;
	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not open default backend");
	/* So we had 2 keys before in the keyset */

	KeySet *ks = ksNew ( 3, KS_END);


	Split *split = elektraSplitNew();

	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should not need sync");
	succeed_if (elektraSplitSync (split) == 0, "should not need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 2, "there is an empty keset");
	succeed_if (ksGetSize(split->keysets[0]) == 0, "wrong size");
	succeed_if (ksGetSize(split->keysets[1]) == 0, "wrong size");

	elektraSplitDel (split);


	handle->defaultBackend->usersize = 2;
	handle->defaultBackend->systemsize = 0;
	split = elektraSplitNew();

	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should not need sync");
	succeed_if (elektraSplitSync (split) == 1, "should not need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 2, "there is an empty keset");
	succeed_if (ksGetSize(split->keysets[0]) == 0, "wrong size");
	succeed_if (ksGetSize(split->keysets[1]) == 0, "wrong size");

	elektraSplitDel (split);


	handle->defaultBackend->usersize = 2;
	handle->defaultBackend->systemsize = 0;
	split = elektraSplitNew();

	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "should not need sync");
	succeed_if (elektraSplitSync (split) == 1, "should not need sync");

	succeed_if (split->keysets, "did not alloc keysets array");
	succeed_if (split->handles, "did not alloc handles array");
	succeed_if (split->size == 2, "there is an empty keset");
	succeed_if (ksGetSize(split->keysets[0]) == 0, "wrong size");
	succeed_if (ksGetSize(split->keysets[1]) == 0, "wrong size");

	elektraSplitPrepare(split);
	succeed_if (split->size == 1, "there is an empty keset");
	succeed_if (!strcmp(keyName(split->parents[0]), "user"), "parent key not correct");
	succeed_if (!strcmp(keyValue(split->parents[0]), "default"), "parent value not correct");

	elektraSplitDel (split);


	ksDel (ks);
	kdb_close(handle);
	keyDel (parent);
}

void test_realworld()
{
	printf ("Test real world example\n");

	Key *parent = 0;
	KDB *handle = kdb_open();

	succeed_if (elektraMountOpen (handle, set_realworld(), handle->modules, 0) == 0, "could not open mountpoints");
	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not open default backend");

	KeySet *ks = ksNew ( 18,
		keyNew ("system/elektra/mountpoints", KEY_END),
		keyNew ("system/elektra/mountpoints/new", KEY_END),
		keyNew ("system/elektra/mountpoints/new/mountpoint", KEY_VALUE, "something", KEY_END),
		keyNew ("system/users", KEY_END),
		keyNew ("system/users/markus", KEY_END),
		keyNew ("system/users/harald", KEY_END),
		keyNew ("system/users/n", KEY_END),
		keyNew ("system/users/albert", KEY_END),
		keyNew ("system/hosts", KEY_END),
		keyNew ("system/hosts/markusbyte", KEY_VALUE, "127.0.0.1", KEY_END),
		keyNew ("system/hosts/mobilebyte", KEY_END),
		keyNew ("system/hosts/n900", KEY_END),
		keyNew ("user/sw/apps/app1/default", KEY_END),
		keyNew ("user/sw/apps/app1/default/maximize", KEY_VALUE, "1", KEY_END),
		keyNew ("user/sw/apps/app1/default/download", KEY_VALUE, "0", KEY_END),
		keyNew ("user/sw/apps/app1/default/keys/a", KEY_VALUE, "a", KEY_END),
		keyNew ("user/sw/apps/app1/default/keys/b", KEY_VALUE, "b", KEY_END),
		keyNew ("user/sw/apps/app1/default/keys/c", KEY_VALUE, "c", KEY_END),
		keyNew ("user/outside", KEY_VALUE, "test", KEY_END),
		KS_END);
	KeySet *split0 = ksNew ( 9,
		keyNew ("system/elektra/mountpoints", KEY_END),
		keyNew ("system/elektra/mountpoints/new", KEY_END),
		keyNew ("system/elektra/mountpoints/new/mountpoint", KEY_VALUE, "something", KEY_END),
		KS_END);
	KeySet *split2 = ksNew ( 9,
		keyNew ("system/hosts", KEY_END),
		keyNew ("system/hosts/markusbyte", KEY_VALUE, "127.0.0.1", KEY_END),
		keyNew ("system/hosts/mobilebyte", KEY_END),
		keyNew ("system/hosts/n900", KEY_END),
		KS_END);
	KeySet *split3 = ksNew ( 9,
		keyNew ("system/users", KEY_END),
		keyNew ("system/users/markus", KEY_END),
		keyNew ("system/users/harald", KEY_END),
		keyNew ("system/users/n", KEY_END),
		keyNew ("system/users/albert", KEY_END),
		KS_END);
	KeySet *split4 = ksNew ( 9,
		keyNew ("user/sw/apps/app1/default", KEY_END),
		keyNew ("user/sw/apps/app1/default/maximize", KEY_VALUE, "1", KEY_END),
		keyNew ("user/sw/apps/app1/default/download", KEY_VALUE, "0", KEY_END),
		keyNew ("user/sw/apps/app1/default/keys/a", KEY_VALUE, "a", KEY_END),
		keyNew ("user/sw/apps/app1/default/keys/b", KEY_VALUE, "b", KEY_END),
		keyNew ("user/sw/apps/app1/default/keys/c", KEY_VALUE, "c", KEY_END),
		KS_END);
	KeySet *split7 = ksNew ( 3, 
		keyNew ("user/outside", KEY_VALUE, "test", KEY_END),
		KS_END);


	Split *split = elektraSplitNew();

	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (split->size == 10, "size of split not correct");
	succeed_if (split->syncbits[0]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[0]) == 0, "wrong size");
	succeed_if (split->syncbits[1]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 0, "wrong size");
	succeed_if (split->syncbits[2]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[2]) == 0, "wrong size");
	succeed_if (split->syncbits[3]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[3]) == 0, "wrong size");
	succeed_if (split->syncbits[4]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[4]) == 0, "wrong size");
	succeed_if (split->syncbits[5]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[5]) == 0, "wrong size");
	succeed_if (split->syncbits[6]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[6]) == 0, "wrong size");
	succeed_if (split->syncbits[7]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[7]) == 0, "wrong size");
	succeed_if (split->syncbits[8]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[8]) == 0, "wrong size");
	succeed_if (split->syncbits[9]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[9]) == 0, "wrong size");

	succeed_if (elektraSplitDivide (split, handle, ks) == 1, "should need sync");
	succeed_if (split->size == 10, "size of split not correct");
	succeed_if (split->syncbits[0]== 1, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[0]) == 6, "wrong size");
	succeed_if (split->syncbits[1]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 0, "wrong size");
	succeed_if (split->syncbits[2]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[2]) == 0, "wrong size");
	succeed_if (split->syncbits[3]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[3]) == 0, "wrong size");
	succeed_if (split->syncbits[4]== 1, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[4]) == 4, "wrong size");
	succeed_if (split->syncbits[5]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[5]) == 0, "wrong size");
	succeed_if (split->syncbits[6]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[6]) == 0, "wrong size");
	succeed_if (split->syncbits[7]== 3, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[7]) == 1, "wrong size");
	succeed_if (split->syncbits[8]== 1, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[8]) == 5, "wrong size");
	succeed_if (split->syncbits[9]== 3, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[9]) == 3, "wrong size");

	split->handles[5]->usersize = 5;
	split->handles[8]->systemsize = 12;
	succeed_if (elektraSplitSync (split) == 1, "should need sync");
	succeed_if (split->size == 10, "size of split not correct");
	succeed_if (split->syncbits[0]== 1, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[0]) == 6, "wrong size");
	succeed_if (split->syncbits[1]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 0, "wrong size");
	succeed_if (split->syncbits[2]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[2]) == 0, "wrong size");
	succeed_if (split->syncbits[3]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[3]) == 0, "wrong size");
	succeed_if (split->syncbits[4]== 1, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[4]) == 4, "wrong size");
	succeed_if (split->syncbits[5]== 1, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[5]) == 0, "wrong size");
	succeed_if (split->syncbits[6]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[6]) == 0, "wrong size");
	succeed_if (split->syncbits[7]== 3, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[7]) == 1, "wrong size");
	succeed_if (split->syncbits[8]== 1, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[8]) == 5, "wrong size");
	succeed_if (split->syncbits[9]== 3, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[9]) == 3, "wrong size");


	split->handles[5]->usersize = 0;
	split->handles[8]->systemsize = 0;
	elektraSplitDel (split);



	clear_sync (ks);
	split = elektraSplitNew();
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (split->size == 10, "size of split not correct");
	succeed_if (split->syncbits[0]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[0]) == 0, "wrong size");
	succeed_if (split->syncbits[1]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 0, "wrong size");
	succeed_if (split->syncbits[2]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[2]) == 0, "wrong size");
	succeed_if (split->syncbits[3]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[3]) == 0, "wrong size");
	succeed_if (split->syncbits[4]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[4]) == 0, "wrong size");
	succeed_if (split->syncbits[5]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[5]) == 0, "wrong size");
	succeed_if (split->syncbits[6]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[6]) == 0, "wrong size");
	succeed_if (split->syncbits[7]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[7]) == 0, "wrong size");
	succeed_if (split->syncbits[8]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[8]) == 0, "wrong size");
	succeed_if (split->syncbits[9]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[9]) == 0, "wrong size");

	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "does not need sync anymore");
	succeed_if (split->size == 10, "size of split not correct");
	succeed_if (split->syncbits[0]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[0]) == 6, "wrong size");
	succeed_if (split->syncbits[1]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 0, "wrong size");
	succeed_if (split->syncbits[2]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[2]) == 0, "wrong size");
	succeed_if (split->syncbits[3]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[3]) == 0, "wrong size");
	succeed_if (split->syncbits[4]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[4]) == 4, "wrong size");
	succeed_if (split->syncbits[5]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[5]) == 0, "wrong size");
	succeed_if (split->syncbits[6]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[6]) == 0, "wrong size");
	succeed_if (split->syncbits[7]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[7]) == 1, "wrong size");
	succeed_if (split->syncbits[8]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[8]) == 5, "wrong size");
	succeed_if (split->syncbits[9]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[9]) == 3, "wrong size");

	succeed_if (elektraSplitSync (split) == 1, "should need sync, because of removes");
	/* We have the same as before, because everywhere were keys has to be deleted now */
	succeed_if (split->size == 10, "size of split not correct");
	succeed_if (split->syncbits[0]== 1, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[0]) == 6, "wrong size");
	succeed_if (split->syncbits[1]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 0, "wrong size");
	succeed_if (split->syncbits[2]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[2]) == 0, "wrong size");
	succeed_if (split->syncbits[3]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[3]) == 0, "wrong size");
	succeed_if (split->syncbits[4]== 1, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[4]) == 4, "wrong size");
	succeed_if (split->syncbits[5]== 0, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[5]) == 0, "wrong size");
	succeed_if (split->syncbits[6]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[6]) == 0, "wrong size");
	succeed_if (split->syncbits[7]== 3, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[7]) == 1, "wrong size");
	succeed_if (split->syncbits[8]== 1, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[8]) == 5, "wrong size");
	succeed_if (split->syncbits[9]== 3, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[9]) == 3, "wrong size");

	elektraSplitDel (split);


	split = elektraSplitNew();
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (split->size == 10, "size not correct");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "does not need sync anymore");
	split->handles[0]->usersize = 6;
	split->handles[4]->systemsize = 4;
	split->handles[7]->usersize = 1;
	split->handles[8]->systemsize = 5;
	split->handles[9]->systemsize = 3;
	succeed_if (elektraSplitSync (split) == 0, "no sync needed");
	elektraSplitDel (split);


	split = elektraSplitNew();
	succeed_if (elektraSplitBuildup (split, handle, parent) == 1, "should need sync");
	succeed_if (split->size == 10, "size not correct");
	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "does not need sync anymore");
	split->handles[0]->usersize = 6;
	split->handles[4]->systemsize = 2; /* Changed */
	split->handles[7]->usersize = 1;
	split->handles[8]->systemsize = 5;
	split->handles[9]->systemsize = 3;
	succeed_if (elektraSplitSync (split) == 1, "sync needed because one size not correct");

	succeed_if( elektraSplitPrepare(split) == 0, "prepare did not work");
	succeed_if (split->size == 1, "size not correct");
	succeed_if (!strcmp(keyName(split->parents[0]), "system/hosts"), "parent key not correct");
	succeed_if (!strcmp(keyValue(split->parents[0]), "hosts"), "parent value not correct");

	elektraSplitDel (split);


	ksDel (ks);
	ksDel (split0);
	ksDel (split2);
	ksDel (split3);
	ksDel (split4);
	ksDel (split7);
	keyDel (parent);
	kdb_close(handle);

}


void test_emptysplit()
{
	printf ("Test empty split\n");

	KDB *handle = kdb_open();
	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not open default backend");

	KeySet *ks = ksNew(0, KS_END);
	Split *split = elektraSplitNew();
	Key *parentKey;

	succeed_if (split->size == 0, "size should be zero");
	succeed_if (split->alloc == APPROXIMATE_NR_OF_BACKENDS, "initial size not correct");

	succeed_if (elektraSplitBuildup (split, handle, 0) == 1, "default backend should be added");
	succeed_if (split->size == 2, "size of split not correct");
	succeed_if (split->syncbits[0]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[0]) == 0, "wrong size");
	succeed_if (split->syncbits[1]== 2, "size of split not correct");
	succeed_if (ksGetSize(split->keysets[1]) == 0, "wrong size");

	parentKey = keyNew ("system", KEY_VALUE, "default", KEY_END);
	compare_key(split->parents[0], parentKey);
	keyDel (parentKey);

	parentKey = keyNew ("user", KEY_VALUE, "default", KEY_END);
	compare_key(split->parents[1], parentKey);
	keyDel (parentKey);

	succeed_if (split->handles[0] == handle->defaultBackend, "not correct backend");
	succeed_if (split->handles[1] == handle->defaultBackend, "not correct backend");
	succeed_if (split->syncbits[0] == 2, "should be marked as default");
	succeed_if (split->syncbits[1] == 2, "should be marked as default");

	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "there should be no added key");

	succeed_if (split->size == 2, "divide never changes size");
	succeed_if (split->alloc == APPROXIMATE_NR_OF_BACKENDS, "initial size not correct");

	elektraSplitDel (split);
	ksDel (ks);
	kdb_close(handle);
}


void test_nothingsync()
{
	printf ("Test buildup with nothing to sync\n");
	KDB *handle = kdb_open();
	succeed_if (elektraMountDefault (handle, handle->modules, 0) == 0, "could not open default backend");

	KeySet *ks = ksNew(0, KS_END);

	Split *split = elektraSplitNew();
	Key *parentKey = keyNew("user", KEY_VALUE, "default", KEY_END);

	succeed_if (elektraSplitBuildup (split, handle, parentKey) == 1, "we add the default backend for user");

	succeed_if (split->size == 1, "there is an empty keset");
	succeed_if (ksGetSize(split->keysets[0]) == 0, "wrong size");
	compare_key(split->parents[0], parentKey);
	succeed_if (split->handles[0] == handle->defaultBackend, "not correct backend");
	succeed_if (split->syncbits[0] == 2, "should be marked as root");

	succeed_if (elektraSplitDivide (split, handle, ks) == 0, "does not need sync anymore");
	succeed_if (elektraSplitSync (split) == 0, "nothing to sync");
	succeed_if( elektraSplitPrepare(split) == 0, "prepare did not work");
	succeed_if (split->size == 0, "there should be nothing to sync");

	elektraSplitDel (split);
	keyDel (parentKey);

	ksDel (ks);
	kdb_close(handle);
}

int main(int argc, char** argv)
{
	printf("SPLIT SET   TESTS\n");
	printf("==================\n\n");

	init (argc, argv);

	test_needsync();
	test_mount();
	test_easyparent();
	test_optimize();
	test_three();
	test_userremove();
	test_systemremove();
	test_emptyremove();
	test_realworld();
	test_emptysplit();
	test_nothingsync();

	printf("\ntest_splitset RESULTS: %d test(s) done. %d error(s).\n", nbTest, nbError);

	return nbError;
}

