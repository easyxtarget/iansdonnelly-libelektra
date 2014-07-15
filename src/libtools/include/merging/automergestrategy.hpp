/**
 * \file
 *
 * \brief A strategy for taking the value of
 *
 * \copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */
#ifndef AUTOMERGESTRATEGY_HPP_
#define AUTOMERGESTRATEGY_HPP_

#include <merging/mergeconflictstrategy.hpp>

namespace kdb
{

namespace tools
{

namespace merging
{

class AutoMergeStrategy : public MergeConflictStrategy
{
public:
	virtual void resolveConflict(const MergeTask& task, Key& conflictKey, MergeResult& result);
};

}
}
}

#endif /* AUTOMERGESTRATEGY_HPP_ */
