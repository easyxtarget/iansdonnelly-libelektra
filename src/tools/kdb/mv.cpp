#include <mv.hpp>

#include <kdb.hpp>
#include <rename.hpp>
#include <cmdline.hpp>
#include <keysetio.hpp>

#include <iostream>

using namespace std;
using namespace kdb;

MvCommand::MvCommand()
{}

int MvCommand::execute (Cmdline const& cl)
{
	if (cl.arguments.size() != 2)
	{
		throw invalid_argument("wrong number of arguments, 2 needed");
	}

	KeySet conf;
	Key sourceKey(cl.arguments[0], KEY_END);
	if (!sourceKey.isValid())
	{
		throw invalid_argument("Source given is not a valid keyname");
	}

	Key destKey(cl.arguments[1], KEY_END);
	if (!destKey.isValid())
	{
		throw invalid_argument("Destination given is not a valid keyname");
	}
	string newDirName = cl.arguments[1];

	kdb.get(conf, sourceKey);
	printWarnings(cerr, sourceKey);
	kdb.get(conf, destKey);
	printWarnings(cerr, destKey);
	KeySet tmpConf = conf;
	KeySet oldConf;

	oldConf.append (tmpConf.cut(sourceKey));

	KeySet newConf;

	Key k;
	oldConf.rewind();
	std::string sourceName = sourceKey.getName();
	if (cl.verbose) cout << "common name: " << sourceName << endl;
	if (cl.recursive)
	{
		while ((k = oldConf.next()))
		{
			newConf.append(rename_key(k, sourceName, newDirName, cl.verbose));
		}
	}
	else
	{
		// just rename one key
		k = oldConf.next();
		if (!k)
		{
			cerr << "Single key to move not found\n";
			return 1;
		}
		if (k != sourceKey)
		{
			cerr << "First key found " << k.getName()
			     << " does not exactly match given key " << sourceKey.getName()
			     << ", aborting (use -r to move hierarchy)\n";
			return 1;
		}
		newConf.append(rename_key(k, sourceName, newDirName, cl.verbose));
	}
	newConf.append(tmpConf); // these are unrelated keys
	// drop the original configuration

	newConf.rewind();
	if (cl.verbose)
	{
		cout << "Will write out:" << endl;
		cout << newConf;
	}

	Key parentKey;
	kdb.set(newConf, parentKey);
	printWarnings(cerr, parentKey);

	return 0;
}

MvCommand::~MvCommand()
{}
