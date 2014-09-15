#include <tests.hpp>

#include <vector>
#include <string>
#include <stdexcept>

void test_null()
{
	cout << "testing null" << endl;

	Key key0(static_cast<ckdb::Key*>(0));
	succeed_if (!key0, "key should evaluate to false");

	key0 = static_cast<ckdb::Key*>(0);
	succeed_if (!key0, "key should evaluate to false");

	key0.release();
	succeed_if (!key0, "key should evaluate to false");
}

void test_keynew()
{
	cout << "testing keynew" << endl;

	char array[] = "here is some data stored";

	Key key0;
	succeed_if( key0, "key should evaluate to true");
	succeed_if( key0.getName() == "", "key0 has wrong name");

	// Empty key
	Key key1 ("", KEY_END);
	succeed_if( key1, "key should evaluate to true");
	succeed_if( key1.getName() == "", "key0 has wrong name");

	// Key with name
	Key key2 ("system/sw/test", KEY_END);
	succeed_if (key2.getBaseName() == "test", "wrong base name");
	succeed_if( key2.getName() == "system/sw/test", "key2 has wrong name");
	succeed_if (key2.getDirName() == "system/sw", "wrong dir name");
	key2.copy(key0);
	succeed_if( key2.getName() == "", "key0 has wrong name");
	succeed_if (key2.getBaseName() == "", "wrong base name");
	succeed_if (key2.getDirName() == "", "wrong dir name");


	// Key with name
	Key key3 ("system/sw/test", KEY_END);
	succeed_if(key3.getName() == "system/sw/test", "key3 has wrong name");
	succeed_if(key3.getBaseName() == "test", "wrong base name");
	succeed_if (key3.getDirName() == "system/sw", "wrong dir name");
	key3.setName("system/other/name");
	succeed_if(key3.getName() == "system/other/name", "key3 has wrong name");
	succeed_if(key3.getBaseName() == "name", "wrong base name");
	succeed_if (key3.getDirName() == "system/other", "wrong dir name");
	key3.addBaseName("base");
	succeed_if(key3.getName() == "system/other/name/base", "key3 has wrong name");
	succeed_if(key3.getBaseName() == "base", "wrong base name");
	succeed_if (key3.getDirName() == "system/other/name", "wrong dir name");
	key3.setBaseName("name");
	succeed_if(key3.getName() == "system/other/name/name", "key3 has wrong name");
	succeed_if(key3.getBaseName() == "name", "wrong base name");
	succeed_if (key3.getDirName() == "system/other/name", "wrong dir name");
	key3.setName("system/name");
	succeed_if(key3.getName() == "system/name", "key3 has wrong name");
	succeed_if(key3.getBaseName() == "name", "wrong base name");
	succeed_if (key3.getDirName() == "system", "wrong dir name");
	/*
	key3.addName("some/more");
	succeed_if(key3.getName() == "system/name/some/more", "key3 has wrong name");
	succeed_if(key3.getBaseName() == "more", "wrong base name");
	succeed_if (key3.getDirName() == "system/name/some", "wrong dir name");
	*/

	// Key with name + value
	Key key4 ("system/sw/test",
			KEY_VALUE, "test",
			KEY_END);
	succeed_if(key4.getName() == "system/sw/test", "key4 has wrong name");
	succeed_if(key4.getString() == "test", "key4 has wrong value");
	succeed_if(key4.get<string>() == "test", "key4 has wrong value");
	succeed_if(key4.getStringSize() == 5, "key4 has wrong value size");
	key4.setString("test");
	succeed_if(key4.getString() == "test", "key4 has wrong value");
	succeed_if(key4.get<string>() == "test", "key4 has wrong value");
	succeed_if(key4.getStringSize() == 5, "key4 has wrong value size");
	try
	{
		key4.getBinary();
		succeed_if (false, "string key did not throw after getting binary");
	}
	catch (KeyTypeMismatch const & ktm)
	{
		succeed_if (true, "string key did not throw after getting binary");
	}

	try
	{
		key4.get<int>();
		succeed_if (false, "string key did not throw after int");
	}
	catch (KeyTypeConversion const & ktm)
	{
		succeed_if (true, "string key did not throw after getting binary");
	}

	key4.setString("");
	succeed_if(key4.getString() == "", "key4 has wrong value");
	succeed_if(key4.isString(), "key4 is not string");
	succeed_if(!key4.isBinary(), "key4 is not string");
	succeed_if(key4.get<string>() == "", "key4 has wrong value");
	succeed_if(key4.getStringSize() == 1, "key4 has wrong value size");
	key4.setBinary("abc", 3);
	succeed_if(key4.isBinary(), "key4 is not string");
	succeed_if(!key4.isString(), "key4 is not string");
	try
	{
		key4.getString();
		succeed_if (false, "binary key did not throw after getting string");
	}
	catch (KeyTypeMismatch const & ktm)
	{
		succeed_if (true, "binary key did not throw after getting string");
	}
	std::string s("abc");
	s.resize(3);
	succeed_if(key4.getBinary() == s, "key4 has wrong binary value");
	succeed_if(key4.getBinarySize() == 3, "key4 has wrong value size");
	s[1] = 0;
	key4.setBinary("a\0c", 3);
	succeed_if(key4.getBinary() == s, "key4 has wrong binary value");
	succeed_if(key4.getBinarySize() == 3, "key4 has wrong value size");

	// Key with name + UID/GID
	Key key5 ("system/sw/test",
			KEY_UID, 123,
			KEY_GID, 456,
			KEY_END);
	succeed_if(key5.getMeta<uid_t>("uid") == 123, "key5 UID no set correctly");
	succeed_if(key5.getMeta<gid_t>("gid") == 456, "key5 UID no set correctly");
	succeed_if(key5.getName() == "system/sw/test", "key5 has wrong name");

	// Key with name + MODE
	Key key6 ("system/sw/test",
			KEY_MODE, 0642,
			KEY_END);
	succeed_if(key6.getMeta<mode_t>("mode") == 642, "key6 mode no set correctly");
	succeed_if(key6.getName() == "system/sw/test", "key6 has wrong name");
	key6.setString("a very long string");
	succeed_if(key6.getString() == "a very long string", "key6 has wrong value");
	succeed_if(key6.get<string>() == "a very long string", "key6 has wrong value");

	// Key with name + owner
	Key key7 ("system/sw/test",
			KEY_OWNER, "yl",
			KEY_END);
	succeed_if( key7.getMeta<std::string>("owner") ==  "yl", "key7 owner not set correctly");
	succeed_if (!key7.isInactive(), "key should not be inactive");

	Key key8 ("system/valid/there",
			KEY_BINARY,
			KEY_SIZE, sizeof(array),
			KEY_VALUE, array,
			KEY_END);
	succeed_if(key8.getName() == "system/valid/there", "key8 has wrong name");
	succeed_if(key8.isBinary (), "Key should be binary");
	succeed_if(!key8.isString(), "Key should be binary");
	succeed_if(key8.getBinarySize() == sizeof(array), "Value size not correct");
	std::string getBack = key8.getBinary();
	succeed_if(memcmp(&getBack[0], array, sizeof(array)) == 0, "could not get correct value with keyGetBinary");
	succeed_if (key8.getBaseName() == "there", "wrong base name");

	Key key9("system/valid/.inactive", KEY_COMMENT, "inactive key", KEY_END);
	succeed_if (key9.isInactive(), "key should be inactive");
	succeed_if (key9.getMeta<std::string>("comment") == "inactive key", "comment failed");
	succeed_if (key9.getBaseName() == ".inactive", "wrong base name");

	std::string name = "system/valid/name";
	Key keyA(name, KEY_END);
	succeed_if (keyA.getName() == "system/valid/name", "keyA has wrong name");
	succeed_if (keyA.getBaseName() == "name", "keyA wrong base name");

	Key keyB("", KEY_END);
	keyB.setBinary(0, 0);
	succeed_if (keyB.isBinary(), "should be binary");
	succeed_if (keyB.getBinary() == "", "Binary should be a nullpointer");
	succeed_if (keyB.getValue() == 0, "Binary should be a nullpointer");

	keyB.setBinary(0, 1);
	succeed_if (keyB.isBinary(), "should be binary");
	succeed_if (keyB.getBinary() == "", "Binary should be a nullpointer");
	succeed_if (keyB.getValue() == 0, "Binary should be a nullpointer");
}

void test_constructor()
{
	cout << "testing constructor" << endl;

	ckdb::Key *ck = ckdb::keyNew(0);
	Key k = ck; // constructor with (ckdb::Key)

	/*
	cout << "ck:   " << (void*)ck << endl;
	cout << "k:    " << (void*)&k << endl;
	cout << "k.ck: " << (void*)k.getKey() << endl;
	*/

	k.set<int>(30);
	succeed_if (k.get<int> () == 30, "could not get same int");
}

void test_setkey()
{
	cout << "testing setkey" << endl;

	ckdb::Key *ck;
	Key k;

	ck = ckdb::keyNew(0);
	k = ck; // operator= alias for setKey()

	/*
	cout << "ck:   " << (void*)ck << endl;
	cout << "k:    " << (void*)&k << endl;
	cout << "k.ck: " << (void*)k.getKey() << endl;
	*/

	k.set<int>(30);
	succeed_if (k.get<int> () == 30, "could not get same int");
}

void test_cast()
{
	cout << "testing cast" << endl;
	ckdb::Key *ck;
	Key *k;

	ck = ckdb::keyNew(0);
	k = (Key*) &ck; // no copy, just a cast

	/*
	cout << "&ck:  " << (void*)&ck << endl;
	cout << "k:    " << (void*)&k << endl;
	cout << "ck:   " << (void*)ck << endl;
	cout << "k.ck: " << (void*)k->getKey() << endl;
	*/

	k->set<int>(30);
	succeed_if (k->get<int> () == 30, "could not get same int");

	ckdb::keyDel (ck);
}

void test_value ()
{
	cout << "testing value" << endl;
	Key test;
	succeed_if (test.getString() == "", "String should be empty");

	test.setString ("23.3");
	succeed_if (test.get<double> () >= 23.2, "could not get same double");
	succeed_if (test.get<double> () <= 23.4, "could not get same double");
	succeed_if (test.getBinarySize () == 5, "value size not correct");

	test.setString ("401");
	succeed_if (test.get<int> () == 401, "could not get same int");
	succeed_if (test.getBinarySize () == 4, "value size not correct");

	test.setString ("mystr");
	succeed_if (test.get<string> () == "mystr", "could not get same string");
	succeed_if (test.getBinarySize () == 6, "value size not correct");

	test.setString ("myoth");
	succeed_if (test.get<string> () == "myoth", "could not get same string");
	succeed_if (test.getBinarySize () == 6, "value size not correct");

	test.set<double> (23.3);
	succeed_if (test.getString() == "23.3", "could not get same double");
	succeed_if (test.getBinarySize () == 5, "value size not correct");

	test.set<int> (401);
	succeed_if (test.getString() == "401", "could not get same int");
	succeed_if (test.getBinarySize () == 4, "value size not correct");

	test.set<string> ("mystr");
	succeed_if (test.getString() == "mystr", "could not get same string");
	succeed_if (test.getBinarySize () == 6, "value size not correct");

	test.set<string> ("myoth");
	succeed_if (test.getString () == "myoth", "could not get same string");
	succeed_if (test.getBinarySize () == 6, "value size not correct");

	test.setMeta<std::string>("comment", "mycomment");
	succeed_if (test.getMeta<std::string>("comment") == "mycomment", "could not get same comment");
}

void test_exceptions ()
{
	cout << "testing exceptions" << endl;
	Key test;

	try {
		test.setName("no");
	} catch (kdb::KeyInvalidName)
	{
		succeed_if (test.getName() == "", "not set to noname");
	}

	test.setName ("user/name");
	succeed_if (test.getName() == "user/name", "could not get same name");

	try {
		test.setName("no");
	} catch (kdb::KeyInvalidName)
	{
		succeed_if (test.getName() == "", "not set to noname");
	}
}

void test_name()
{
	cout << "testing name" << endl;

	Key test;
	succeed_if (test.getName() == "", "Name should be empty");

	test.setName("user:markus/test");
	succeed_if (test.getName() == "user/test", "Wrong name");
	succeed_if (test.getFullName() == "user:markus/test", "Wrong full name");
	succeed_if (test.getMeta<std::string>("owner") == "markus", "Wrong owner");
	succeed_if (test.getNameSize() == 10, "wrong name size");
	succeed_if (test.getFullNameSize() == 17, "wrong full name size");
	succeed_if (!test.isSystem(), "key is system");
	succeed_if ( test.isUser(), "key is not user");

	test.setMeta<std::string>("owner", "gerald");
	succeed_if (test.getName() == "user/test", "Wrong name");
	succeed_if (test.getFullName() == "user:gerald/test", "Wrong full name");
	succeed_if (test.getMeta<std::string>("owner") == "gerald", "Wrong owner");
	succeed_if (test.getNameSize() == 10, "wrong name size");
	succeed_if (test.getFullNameSize() == 17, "wrong full name size");
	succeed_if (!test.isSystem(), "key is system");
	succeed_if ( test.isUser(), "key is not user");

	test.setName("system/test");
	test.setMeta<std::string>("owner", "markus");
	succeed_if (test.getName() == "system/test", "Wrong name");
	succeed_if (test.getFullName() == "system/test", "Wrong full name");
	succeed_if (test.getMeta<std::string>("owner") == "markus", "Wrong owner");
	succeed_if (test.getMeta<std::string>("owner") == "markus", "Wrong owner");
	succeed_if (test.getNameSize() == 12, "wrong name size");
	succeed_if (test.getFullNameSize() == 12, "wrong full name size");
	succeed_if ( test.isSystem(), "key is system");
	succeed_if (!test.isUser(), "key is not user");

	test.setName("user/dir/test");
	test.setBaseName ("mykey");
	succeed_if (test.getName() == "user/dir/mykey", "Basename did not work");
	test.setName (test.getName() + "/onedeeper"); // add basename is trivial
	succeed_if (test.getName().find('/') == 4, "user length"); // keyGetRootNameSize trivial

	// so we finally got a name, lets test below
	succeed_if (test.getName() == "user/dir/mykey/onedeeper", "Basename did not work");

	succeed_if (test.isBelow (Key("user", KEY_END)), "key is below");
	succeed_if (test.isBelow (Key("user/dir", KEY_END)), "key is below");
	succeed_if (test.isBelow (Key("user/dir/mykey", KEY_END)), "key is below");
	succeed_if (!test.isBelow (Key("user/dir/mykey/onedeeper", KEY_END)), "key is not below (but same)");
	succeed_if (!test.isBelow (Key("user/otherdir", KEY_END)), "key is not below");

	succeed_if (test.isBelowOrSame (Key("user", KEY_END)), "key is below");
	succeed_if (test.isBelowOrSame (Key("user/dir", KEY_END)), "key is below");
	succeed_if (test.isBelowOrSame (Key("user/dir/mykey", KEY_END)), "key is below");
	succeed_if (test.isBelowOrSame (Key("user/dir/mykey/onedeeper", KEY_END)), "key is same");
	succeed_if (!test.isBelowOrSame (Key("user/otherdir", KEY_END)), "key is not below");

	succeed_if (test.isDirectBelow (Key("user/dir/mykey", KEY_END)), "key is direct below");
	succeed_if (!test.isDirectBelow (Key("user/dir/test", KEY_END)), "key is not direct below");
	succeed_if (!test.isDirectBelow (Key("user/dir", KEY_END)), "key is not direct below");
	succeed_if (!test.isDirectBelow (Key("user/dir/otherdir", KEY_END)), "key is not direct below");
	succeed_if (!test.isDirectBelow (Key("user/otherdir", KEY_END)), "key is not direct below");
	succeed_if (!test.isDirectBelow (Key("user", KEY_END)), "key is not direct below");
}

void f(Key)
{
	Key h ("user/infunction", KEY_END);
}

void test_ref()
{
	cout << "testing ref" << endl;

	Key zgr1 ("user/zgr1", KEY_END);
	{
		Key zgr2 ("user/zgr2", KEY_END);
		Key zgr3 ("user/zgr3", KEY_END);
		Key zgr4 ("user/zgr4", KEY_END);
		Key zgr5 ("user/zgr5", KEY_END);
		zgr2=zgr1;
		zgr3=zgr1;
		zgr4=zgr1;
	}

	f(zgr1);
	f(Key ("user/passed", KEY_END));

	Key test;
	test.setName("user:markus/test");

	Key ref1;
	ref1 = test; // operator =
	succeed_if(*ref1 == *test, "should point to the same object");

	succeed_if (test.getName() == "user/test", "wrong name");
	succeed_if (ref1.getName() == "user/test", "ref key wrong name");

	Key ref2 = test; // copy constructor
	succeed_if(*ref2 == *test, "should point to the same object");

	succeed_if (test.getName() == "user/test", "wrong name");
	succeed_if (ref2.getName() == "user/test", "ref key wrong name");

	const Key consttest ("user/test", KEY_END);
	Key ref3 = consttest; // const copy constructor
	succeed_if(*ref3 == *consttest, "should point to the same object");

	succeed_if (consttest.getName() == "user/test", "wrong name");
	succeed_if (ref3.getName() == "user/test", "ref key wrong name");
}

void test_dup()
{
	cout << "testing dup" << endl;

	Key test;
	test.setName("user:markus/test");

	Key dup0 = test.dup(); // directly call of dup()

	succeed_if (test.getName() == "user/test", "wrong name");
	succeed_if (dup0.getName() == "user/test", "dup key wrong name");

	Key dup1 = test.dup(); // directly call of dup()
	succeed_if (dup1.getName() == "user/test", "dup key wrong name");

	succeed_if(*test != *dup0, "should be other key")
	succeed_if(*test != *dup1, "should be other key")
}

void test_valid()
{
	cout << "Test if keys are valid or not" << endl;

	Key i1;
	succeed_if (!i1.isValid(), "key should not be valid");
	succeed_if (i1, "even though it is invalid, it is still not a null key");

	Key i2 ("", KEY_END);
	succeed_if (!i2.isValid(), "key should not be valid");
	succeed_if (i2, "even though it is invalid, it is still not a null key");

	vector<string> invalid_names;
	invalid_names.push_back ("/abc");
	invalid_names.push_back ("use");
	invalid_names.push_back ("syste");
	invalid_names.push_back ("error/somthing");
	invalid_names.push_back ("/");
	invalid_names.push_back (".");
	invalid_names.push_back ("..");

	for (size_t i = 0; i<invalid_names.size(); ++i)
	{
		Key i3 (invalid_names[i], KEY_END);
		succeed_if (!i3.isValid(), "key should not be valid");
		succeed_if (i3, "even though it is invalid, it is still not a null key");
	}

	Key v1("user", KEY_END);
	succeed_if (v1.isValid(), "key should be valid");
	succeed_if (v1, "should be non-null too");

	Key v2("system", KEY_END);
	succeed_if (v2.isValid(), "key should be valid");
	succeed_if (v2, "should be non-null too");

	vector<string> valid_names;
	valid_names.push_back ("user/abc");
	valid_names.push_back ("user/s");
	valid_names.push_back ("system/s");
	valid_names.push_back ("user/error/somthing");
	valid_names.push_back ("system/");
	valid_names.push_back ("user/.");
	valid_names.push_back ("system/abc/..");
	valid_names.push_back ("system/abc/../more");

	for (size_t i = 0; i<valid_names.size(); ++i)
	{
		Key v3 (valid_names[i], KEY_END);
		succeed_if (v3.isValid(), "key should be valid");
		succeed_if (v3, "should not be a null key");
	}
}

void test_clear()
{
	cout << "Test clearing of keys" << endl;

	Key k1("user", KEY_END);
	Key k2 = k1;
	Key k3 = k1;

	succeed_if(k1.isValid(), "key should be valid");
	succeed_if(k2.isValid(), "key should be valid");
	succeed_if(k3.isValid(), "key should be valid");

	succeed_if(k1.getName() == "user", "name should be user");
	succeed_if(k2.getName() == "user", "name should be user");
	succeed_if(k3.getName() == "user", "name should be user");


	k1.clear();

	succeed_if(!k1.isValid(), "key should be invalid");
	succeed_if(!k2.isValid(), "key should be invalid");
	succeed_if(!k3.isValid(), "key should be invalid");

	succeed_if(k1.getName() == "", "name should be empty");
	succeed_if(k2.getName() == "", "name should be empty");
	succeed_if(k3.getName() == "", "name should be empty");

	k1.setMeta("test_meta", "meta_value");
	succeed_if(k1.getMeta<std::string>("test_meta") == "meta_value", "metadata not set correctly");
	succeed_if(k2.getMeta<std::string>("test_meta") == "meta_value", "metadata not set correctly");
	succeed_if(k3.getMeta<std::string>("test_meta") == "meta_value", "metadata not set correctly");

	k2.clear();

	succeed_if(!k1.getMeta<const Key>("test_meta"), "metadata not set correctly");
	succeed_if(!k2.getMeta<const Key>("test_meta"), "metadata not set correctly");
	succeed_if(!k3.getMeta<const Key>("test_meta"), "metadata not set correctly");
}

void test_cconv()
{
	cout << "Test conversion to C Key" << endl;

	Key k1("user", KEY_END);
	ckdb::Key * ck1 = k1.getKey();
	succeed_if(!strcmp(ckdb::keyName(ck1), "user"), "c key does not have correct name");
	succeed_if(!strcmp(ckdb::keyName(*k1), "user"), "c key does not have correct name");

	ck1 = k1.release();
	succeed_if(!strcmp(ckdb::keyName(ck1), "user"), "c key does not have correct name");
	ckdb::keyDel (ck1);
}

int main()
{
	cout << "KEY CLASS TESTS" << endl;
	cout << "===============" << endl << endl;

	test_null();
	test_constructor();
	test_setkey();
	test_cast();

	test_keynew();
	test_name();
	test_value();
	test_exceptions();
	test_dup();
	test_ref();
	test_valid();
	test_clear();
	test_cconv();

	cout << endl;
	cout << "testcpp_key RESULTS: " << nbTest << " test(s) done. " << nbError << " error(s)." << endl;
	return nbError;
}
