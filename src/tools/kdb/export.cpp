#include <export.hpp>

#include <kdb.hpp>
#include <modules.hpp>
#include <cmdline.hpp>
#include <toolexcept.hpp>

#include <iostream>

using namespace std;
using namespace kdb;
using namespace kdb::tools;

ExportCommand::ExportCommand()
{}

int ExportCommand::execute(Cmdline const& cl)
{
	size_t argc = cl.arguments.size();
	if (argc != 1 && argc != 2 && argc != 3)
	{
		throw invalid_argument("need 1 to 3 arguments");
	}

	Key root (cl.arguments[0], KEY_END);
	if (!root.isValid())
	{
		throw invalid_argument ("root key is not a valid key name");
	}

	kdb.get(ks, root);
	printWarnings(cerr, root);

	KeySet part (ks.cut(root));

	if (cl.withoutElektra)
	{
		Key systemElektra("system/elektra", KEY_END);
		part.cut(systemElektra);
	}

	string format = "dump";
	if (argc > 1) format = cl.arguments[1];

	string file = "/dev/stdout";
	if (argc > 2 && cl.arguments[2] != "-") file = cl.arguments[2];

	Modules modules;
	PluginPtr plugin = modules.load(format);

	Key errorKey(root);
	errorKey.setString(file);

	plugin->set(part, errorKey);

	printWarnings(cerr, errorKey);
	printError(cerr, errorKey);

	return 0;
}

ExportCommand::~ExportCommand()
{}
