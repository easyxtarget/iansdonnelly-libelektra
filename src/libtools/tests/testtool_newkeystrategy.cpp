/**
 * \file
 *
 * \brief Tests for the NewKeyStrategy
 *
 * \copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#include <gtest/gtest.h>
#include <merging/newkeystrategy.hpp>
#include "mergetestutils.cpp"

using namespace std;
using namespace kdb;
using namespace kdb::tools::merging;

class NewKeyStrategyTest : public MergeTest
{
protected:
	NewKeyStrategy strategy;
	MergeResult result;
	MergeTask task;
	KeySet conflicts;

	NewKeyStrategyTest() : task (MergeTask (BaseMergeKeys (base, baseParent),
				OurMergeKeys (ours, ourParent),
				TheirMergeKeys (theirs, theirParent),
				mergeParent))
	{
		result = MergeResult(conflicts, mergeKeys);
	}
};

TEST_F(NewKeyStrategyTest, AddEqualsKeyMerge)
{
	Key addedKey = Key ("user/parento/config/key5", KEY_VALUE, "value5", KEY_END);
	task.ours.append (addedKey);
	mergeKeys.append (mk5);
	Key conflictKey = mergeKeys.lookup (mk5);
	result.addConflict (conflictKey, ADD, SAME);
	conflictKey = result.getConflictSet ().at (0);

	strategy.resolveConflict (task, conflictKey, result);

	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";
	KeySet merged = result.getMergedKeys ();
	EXPECT_EQ(5, merged.size ());
	compareAllKeys (merged);
}

TEST_F(NewKeyStrategyTest, EqualsAddKeyMerge)
{
	Key addedKey = Key ("user/parentt/config/key5", KEY_VALUE, "value5", KEY_END);
	task.theirs.append (addedKey);
	mergeKeys.append (mk5);
	Key conflictKey = mergeKeys.lookup (mk5);
	result.addConflict (conflictKey, SAME, ADD);
	conflictKey = result.getConflictSet ().at (0);

	strategy.resolveConflict (task, conflictKey, result);

	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";
	KeySet merged = result.getMergedKeys ();
	EXPECT_EQ(5, merged.size ());
	compareAllKeys (merged);
}
