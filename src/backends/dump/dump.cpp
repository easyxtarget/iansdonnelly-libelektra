/***************************************************************************
            dump.c  -  Skeleton of backends to access the Key Database
                             -------------------
    begin                : Mon May  3 15:22:44 CEST 2010
    copyright            : by Markus Raab
    email                : elektra@markus-raab.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

#include "dump.hpp"

extern "C" {

void serialize(std::ostream &os, ckdb::KeySet *ks)
{
	ckdb::Key *cur;

	os << "ksNew " << ckdb::ksGetSize(ks) << std::endl;

	ksRewind(ks);
	while ((cur = ksNext(ks)) != 0)
	{
		size_t namesize = ckdb::keyGetNameSize(cur);
		size_t valuesize = ckdb::keyGetValueSize(cur);
		os << "keyNew " << namesize
		   << " " << valuesize << std::endl;
		os.write(ckdb::keyName(cur), namesize);
		os.write(static_cast<const char*>(ckdb::keyValue(cur)), valuesize);
		os << std::endl;

		const char *metaname;
		const char *metavalue;
		ckdb::keyRewindMeta(cur);
		while ((metaname = ckdb::keyNextMeta(cur)) != 0)
		{
			metavalue = ckdb::keyCurrentMeta(cur);
			size_t metanamesize = ckdb::kdbiStrLen(metaname);
			size_t metavaluesize = ckdb::kdbiStrLen(metavalue);

			os << "keyMeta " << metanamesize
			   << " " << metavaluesize << std::endl;
			os.write (metaname, metanamesize);
			os.write (metavalue, metavaluesize);
			os << std::endl;
		}
		os << "keyEnd" << std::endl;
	}
	os << "ksEnd" << std::endl;
}

void unserialize(std::istream &is, ckdb::KeySet *ks)
{
	ckdb::Key *cur = 0;

	std::vector<char> namebuffer(4048);
	std::vector<char> valuebuffer(4048);
	std::string line;
	std::string command;
	size_t nrKeys;
	size_t namesize;
	size_t valuesize;

	while(std::getline (is, line))
	{
		std::stringstream ss (line);
		ss >> command;

		if (command == "ksNew")
		{
			ss >> nrKeys;

			ksClear(ks);
		}
		else if (command == "keyNew")
		{
			cur = ckdb::keyNew(0);

			ss >> namesize;
			ss >> valuesize;

			if (namesize > namebuffer.size()) namebuffer.resize(namesize);
			is.read(&namebuffer[0], namesize);
			namebuffer[namesize] = 0;
			ckdb::keySetName(cur, &namebuffer[0]);

			if (valuesize > valuebuffer.size()) valuebuffer.resize(valuesize);
			is.read(&valuebuffer[0], valuesize);
			ckdb::keySetRaw (cur, &valuebuffer[0], valuesize);
			std::getline (is, line);
		}
		else if (command == "keyMeta")
		{
			ss >> namesize;
			ss >> valuesize;

			if (namesize > namebuffer.size()) namebuffer.resize(namesize);
			is.read(&namebuffer[0], namesize);
			namebuffer[namesize] = 0;

			if (valuesize > valuebuffer.size()) valuebuffer.resize(valuesize);
			is.read(&valuebuffer[0], valuesize);

			keySetMeta (cur, &namebuffer[0], &valuebuffer[0]);
			std::getline (is, line);
		}
		else if (command == "keyEnd")
		{
			ksAppendKey(ks, cur);
			cur = 0;
		}
		else if (command == "ksEnd")
		{
			break;
		} else {
			std::cerr << "unkown command: " << command << std::endl;
		}
	}
}

int kdbOpen_dump(ckdb::KDB *handle)
{
	int errnosave = errno;
	ckdb::KeySet *ks;
	ckdb::Key *k;

	KDBCap * cap = ckdb::kdbhGetCapability(handle);
	cap->onlyFullGet=1;
	cap->onlyRemoveAll=1;
	cap->onlyAddKeys=1;
	cap->onlyFullSet=1;
	cap->onlySystem=1;
	cap->onlyUser=1;

	ks = ckdb::kdbhGetConfig (handle);
	ckdb::ksRewind (ks);
	while ((k = ckdb::ksNext (ks)) != 0)
	{
		const char *name;
		std::string f;
		size_t pos;

		name = ckdb::keyName(k);
		if (!name) continue;
		f = std::string(name);
		pos = f.find_last_of('/');
		std::string postfix = f.substr(pos);
		if (postfix == "/path") {
			ckdb::kdbhSetBackendData (handle, new std::string((char*)ckdb::keyValue(k)));
		}
	}
	if (!ckdb::kdbhGetBackendData (handle)) ckdb::kdbhSetBackendData (handle, new std::string(DUMP_PATH));

	errno = errnosave;
	return 0;
}

int kdbClose_dump(ckdb::KDB *handle)
{
	int errnosave = errno;

	delete static_cast<std::string*>(ckdb::kdbhGetBackendData (handle));

	errno = errnosave;
	return 0; /* success */
}

ssize_t kdbGet_dump(ckdb::KDB *handle, ckdb::KeySet *returned, const ckdb::Key *parentKey)
{
	ssize_t nr_keys = 0;
	int errnosave = errno;

	if (strcmp (keyName(kdbhGetMountpoint(handle)), keyName(parentKey))) return 0;

	std::ifstream ofs(static_cast<std::string*>(ckdb::kdbhGetBackendData (handle))->c_str());
	unserialize (ofs, returned);

	errno = errnosave;
	return nr_keys; /* success */
}

ssize_t kdbSet_dump(ckdb::KDB *handle, ckdb::KeySet *returned, const ckdb::Key *parentKey)
{
	ssize_t nr_keys = 0;
	int errnosave = errno;

	if (strcmp (keyName(kdbhGetMountpoint(handle)), keyName(parentKey))) return 0;

	std::ofstream ifs(static_cast<std::string*>(ckdb::kdbhGetBackendData (handle))->c_str());
	serialize (ifs, returned);

	errno = errnosave;
	return nr_keys;
}

ckdb::KDB *KDBEXPORT(dump)
{
	return kdbBackendExport(BACKENDNAME,
		KDB_BE_OPEN,	&kdbOpen_dump,
		KDB_BE_CLOSE,	&kdbClose_dump,
		KDB_BE_GET,	&kdbGet_dump,
		KDB_BE_SET,	&kdbSet_dump,
		KDB_BE_VERSION,        BACKENDVERSION,
		KDB_BE_AUTHOR,	"Full Name <email@libelektra.org>",
		KDB_BE_LICENCE,	"BSD",
		KDB_BE_DESCRIPTION,
			"Add description here",
		KDB_BE_END);
}

} // extern C
