#include <kdb.h>
#include <stdio.h>

void f (const Key * source)
{
	Key * dup = keyDup (source);
	printf ("\tin f\n");

	keyDel (dup);
}

void g (const Key * source, KeySet * ks)
{
	Key * dup = keyDup (source);
	printf ("\tin g\n");

	ksAppendKey (ks, dup);
}

void h (Key *k)
{
	Key * c = keyNew("user/from/h", KEY_END);
	printf ("\tin h\n");

	keyCopy (k, c);
	keyDel (c);
	/* the caller will see the changed key k */
}

int main()
{
	Key * origKey;
	KeySet * ks = ksNew(0, KS_END);

	Key * key = keyNew ("user/test/name",
			KEY_VALUE, "myvalue",
			KEY_END);
	printf ("Created key %s with value %s\n",
			keyName(key), keyString(key));

	f(key);
	printf ("Key is unchanged with value %s\n",
			keyString(key));

	g(key, ks);
	printf ("A duplication was appended in keyset with name %s\n",
			keyName(ksHead(ks)));

	h(key);
	printf ("Key has changed to name %s with value %s\n",
			keyName(key), keyString(key));

	/* key is yet independent */
	keyDel (key);

	ksRewind (ks);
	origKey = ksNext (ks);
	key = keyDup (origKey);
	printf ("A duplication of the key %s with value %s\n",
			keyName(key), keyString(key));

	keyDel (key);
	ksDel (ks);
	return 0;
}
