/**
 * \file
 *
 * \brief Implementation of all exceptions elektratools library might throw
 *
 * \copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#ifndef TOOLS_EXCEPTION_HPP
#define TOOLS_EXCEPTION_HPP

#include <stdexcept>
#include <memory>

#include <kdbio.hpp>

namespace kdb
{

/**
 * @brief This namespace is for the libtool library.
 *
 * @note You have to link against libelektratools if you want to
 * use functionality from it. Contrary to classes in namespace kdb it
 * is not header-only.
 *
 * @see Backend for an entry point
 */
namespace tools
{

/**
 * @brief All exceptions from the elektratools library are derived from
 * this exception
 */
struct ToolException : public std::runtime_error
{
	ToolException() :
			runtime_error(
		"When you read this, that means there was something wrong with Elektra Tools.\n"
		"Seems like a wrong exception was thrown."
				)
	{};
	ToolException(std::string message) :
			runtime_error(message)
	{};
};

struct PluginCheckException : public ToolException
{
	virtual const char* what() const throw()
	{
		return  "When you read this, that means there was something wrong with the plugin.\n"
			"Seems like a check could not specify the error any further";
	}
};

struct BackendCheckException : public ToolException
{
	virtual const char* what() const throw()
	{
		return  "When you read this, that means there was something wrong with the backend.\n"
			"Seems like a check could not specify the error any further";
	}
};

struct FileNotValidException : public BackendCheckException
{
	virtual const char* what() const throw()
	{
		return  "The path/mountpoint combination does not work this way.\n"
			"Try to add another path or mountpoint instead.\n"
			"\n"
			"Filenames for cascading and user mountpoints\n"
			"are not allowed to be absolute (starting with /),\n"
			"because user configuration files are by definition\n"
			"different files per-user.\n"
			"\n"
			"If you want to mount an absolute filename, mount\n"
			"it into system/ regardless if it is /etc or somewhere\n"
			"else. Note that the file permissions will apply, so\n"
			"it might be possible for non-root to modify this path\n"
			"of the system-hierarchy.\n"
			;
	}
};

struct MountpointInvalidException : public BackendCheckException
{
	virtual const char* what() const throw()
	{
		return  "Given mountpoint is not a valid keyname, will abort\n"
			"Examples: system/hosts or user/sw/app";
	}
};

struct PluginAlreadyInserted: public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return  "It is not allowed to insert the same plugin again!\n"
			"Try to add other plugins instead.";
	}
};

struct BadPluginName : public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return  "You entered a bad name for a plugin!\n"
			"A valid name of a plugin has either no #\n"
			"or of the following form: #modulename#label# or #ref\n"
			"where ref must be one of the previously defined labels";
	}
};

struct TooManyPlugins : public PluginCheckException
{
	TooManyPlugins(std::string str) :
		m_str(str)
	{}

	virtual ~TooManyPlugins() throw()
	{}

	virtual const char* what() const throw()
	{
		return m_str.c_str();
	}

	std::string m_str;
};

struct OrderingViolation: public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return  "Ordering Violation!\n"
			"You tried to add a plugin which requests another plugin to be positioned first.\n"
			"Please position the other plugin first and try again.";
	}
};

struct ConflictViolation: public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return  "Conflict Violation!\n"
			"You tried to add a plugin which conflicts with another.\n"
			"Please don't add a plugin which conflicts.";
	}
};


struct NoPlugin : public PluginCheckException
{
	NoPlugin(Key key) :
		m_key(key),
		m_str()
	{}

	virtual ~NoPlugin() throw()
	{}

	virtual const char* what() const throw()
	{
		if (m_str.empty())
		{
			// note that the code will be re-evaluated
			// if it prints nothing, but an expensive
			// function not printing anything seems
			// to be unlikely.
			//
			// note that printError/printWarning will be
			// used either from namespace kdb or global
			// namespace.
			std::stringstream ss;
			ss << "Was not able to load such a plugin!\n\n";
			ss << "Maybe you misspelled it, there is no such plugin or the loader has problems.\n";
			ss << "You might want to try to set LD_LIBRARY_PATH, use kdb-full or kdb-static.\n";
			ss << "Errors/Warnings during loading were:\n";
			printError(ss, m_key);
			printWarnings(ss, m_key);
			m_str = ss.str();
		}
		return m_str.c_str();
	}
private:
	Key m_key;
	mutable std::string m_str;
};

struct ReferenceNotFound: public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return  "Could not find a reference!\n"
			"Seems you forgot to create the reference before using it.\n"
			"Use #modulename#label# before you #ref to it.";
	}
};

struct MissingNeeded : public PluginCheckException
{
	std::string need;
	MissingNeeded (std::string const& need_) :
		need(need_)
	{}
	~MissingNeeded () throw()
	{}
	virtual const char* what() const throw()
	{
		return std::string(std::string("The plugin ") + need + " is needed by this plugin but it is not provided.").c_str();
	}
};

struct MissingSymbol: public PluginCheckException
{
	std::string symbol;
	MissingSymbol (std::string const& symbol_) :
		symbol(symbol_)
	{}
	~MissingSymbol () throw()
	{}
	virtual const char* what() const throw()
	{
		// TODO: not safe return value
		return std::string(std::string("The necessary symbol \"") + symbol + "\" is missing in that plugin!").c_str();
	}
};

struct SymbolMismatch: public PluginCheckException
{
	std::string symbol;
	SymbolMismatch (std::string const& symbol_) :
		symbol(symbol_)
	{}
	~SymbolMismatch () throw()
	{}
	virtual const char* what() const throw()
	{
		// TODO: not safe return value
		return std::string(std::string("The symbol \"") + symbol + "\" does not match with other exported information!").c_str();
	}
};

struct SymbolDuplicate: public PluginCheckException
{
	std::string symbol;
	SymbolDuplicate(std::string const& symbol_) :
		symbol(symbol_)
	{}
	~SymbolDuplicate () throw()
	{}
	virtual const char* what() const throw()
	{
		// TODO: not safe return value
		return std::string(std::string("The symbol \"") + symbol + "\" has the same value as another symbol!").c_str();
	}
};

struct StoragePlugin : public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return "There need to be exactly one storage plugin!";
	}
};


struct ResolverPlugin : public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return "There need to be exactly one resolver plugin!";
	}
};

struct PluginWrongName : public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return "The real name of the plugin is different!";
	}
};

struct PluginNoContract: public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return "No contract found for that plugin!\n"
			"Make sure you export kdbGet correctly!";
	}
};

struct PluginNoInfo: public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return "No info found for that plugin within contract!";
	}
};

struct VersionInfoMismatch: public PluginCheckException
{
	virtual const char* what() const throw()
	{
		return "Version info does not match with library!";
	}
};

}

}

#endif
