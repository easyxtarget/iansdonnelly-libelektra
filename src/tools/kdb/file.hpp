#ifndef FILE_HPP
#define FILE_HPP

#include <command.hpp>

#include <kdb.hpp>

class FileCommand : public Command
{
	kdb::KDB kdb;

public:
	FileCommand();
	~FileCommand();

	virtual std::string getShortOptions()
	{
		return "n";
	}

	virtual std::string getSynopsis()
	{
		return "<name>";
	}

	virtual std::string getShortHelpText()
	{
		return "Prints the file where a key is located.";
	}

	virtual std::string getLongHelpText()
	{
		return "While elektra allows one to store configuration in binary\n"
		       "key databases, there are typically stored in configuration\n"
		       "files.\n"
		       "For administrative users and people used to configuration\n"
		       "files it is of interest where the actual configuration file\n"
		       "is located.\n"
		       "This tool outputs where the configuration file is, where\n"
		       "keys would be read from.";
	}

	virtual int execute (Cmdline const& cmdline);
};

#endif
