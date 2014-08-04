#ifndef EXPORT_H
#define EXPORT_H

#include <command.hpp>
#include <kdb.hpp>

class ExportCommand : public Command
{
	kdb::KDB kdb;
	kdb::KeySet ks;

public:
	ExportCommand();
	~ExportCommand();

	virtual std::string getShortOptions()
	{
		return "E";
	}

	virtual std::string getSynopsis()
	{
		return "<source> [<format>]";
	}

	virtual std::string getShortHelpText()
	{
		return "Export configuration from the key database.";
	}

	virtual std::string getLongHelpText()
	{
		return
			"The export utility allows you to export\n"
			"all or parts of the configuration to stdout.\n"
			"\n"
			"Example:\n"
			"kdb export system/sw > sw.ecf\n"
			"To make a backup of your whole configuration\n"
			"below system/sw"
			;
	}

	virtual int execute (Cmdline const& cmdline);
};

#endif
