#ifndef MERGETOOLS_HPP
#define MERGETOOLS_HPP

#include <kdb.hpp>
#include <toolexcept.hpp>
#include <string>

namespace kdb{

namespace tools{

class Merge
{

public:	

	Merge();
	~Merge();
	static std::string getPath(const kdb::Key&);
	static bool KeyEqual(const kdb::Key&, const kdb::Key&);
	static bool KeySetEqual(const kdb::KeySet&, const kdb::KeySet&);
	static KeySet KeySetMerge(const kdb::KeySet&, const kdb::KeySet&, const kdb::KeySet&, kdb::Key&);
	static KeySet KeySetMerge(const kdb::KeySet&, const kdb::Key&, const kdb::KeySet&, const kdb::Key&, const kdb::KeySet&, const kdb::Key&,  kdb::Key&);

};

}

}

#endif
