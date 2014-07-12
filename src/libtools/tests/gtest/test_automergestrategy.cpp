/**
 * \file
 *
 * \brief Tests for the AutoMergeStrategy
 *
 * \copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#include <gtest/gtest.h>
#include <merging/automergestrategy.hpp>
#include "mergetestutils.cpp"

using namespace kdb;
using namespace kdb::tools::merging;

class AutoMergeStrategyTest : public MergeTest
{
protected:
	AutoMergeStrategy strategy;
	MergeResult result;
	MergeTask task;
	KeySet conflicts;

	AutoMergeStrategyTest() : task (MergeTask (BaseMergeKeys (base, baseParent),
				OurMergeKeys (ours, ourParent),
				TheirMergeKeys (theirs, theirParent),
				mergeParent))
	{
		result = MergeResult(conflicts, mergeKeys);
	}
};

TEST_F(AutoMergeStrategyTest, DeleteEqualsMerges)
{
	task.ours.lookup ("user/parento/config/key1", KDB_O_POP);
	Key conflictKey = mergeKeys.lookup (mk1);
	result.addConflict (conflictKey, DELETE, SAME);
	conflictKey = result.getConflictSet ().at (0);

	strategy.resolveConflict (task, conflictKey, result);

	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";
	KeySet merged = result.getMergedKeys ();
	cout << merged << endl;
	EXPECT_EQ(3, merged.size ());

	/* key with index 1 should be deleted */
	compareAllExceptKey1 (merged);
}

TEST_F(AutoMergeStrategyTest, EqualsDeleteMerges)
{
	task.theirs.lookup ("user/parentt/config/key1", KDB_O_POP);
	Key conflictKey = mergeKeys.lookup (mk1);
	result.addConflict (conflictKey, SAME, DELETE);
	conflictKey = result.getConflictSet ().at (0);

	strategy.resolveConflict (task, conflictKey, result);

	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";
	KeySet merged = result.getMergedKeys ();
	EXPECT_EQ(3, merged.size ());

	/* key with index 1 should be deleted */
	compareAllExceptKey1 (merged);
}

TEST_F(AutoMergeStrategyTest, EqualsModifyMerges)
{
	task.theirs.lookup ("user/parentt/config/key1").setString ("modifiedvalue");
	Key conflictKey = mergeKeys.lookup (mk1);
	result.addConflict (conflictKey, SAME, MODIFY);
	conflictKey = result.getConflictSet ().at (0);

	strategy.resolveConflict (task, conflictKey, result);

	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";
	KeySet merged = result.getMergedKeys ();
	EXPECT_EQ(4, merged.size ());

	EXPECT_EQ(mk1, merged.lookup (mk1));
	EXPECT_EQ ("modifiedvalue", merged.lookup (mk1).getString()) << "Key " << merged.lookup (mk1)
			<< "was not modified correctly";

	compareAllExceptKey1 (merged);
}

TEST_F(AutoMergeStrategyTest, ModifyEqualsMerges)
{
	task.ours.lookup ("user/parento/config/key1").setString ("modifiedvalue");
	Key conflictKey = mergeKeys.lookup (mk1);
	result.addConflict (conflictKey, MODIFY, SAME);
	conflictKey = result.getConflictSet ().at (0);

	strategy.resolveConflict (task, conflictKey, result);

	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";
	KeySet merged = result.getMergedKeys ();
	EXPECT_EQ(4, merged.size ());

	EXPECT_EQ(mk1, merged.lookup (mk1));
	EXPECT_EQ ("modifiedvalue", merged.lookup (mk1).getString()) << "Key " << merged.lookup (mk1)
			<< "was not modified correctly";

	compareAllExceptKey1 (merged);
}

TEST_F(AutoMergeStrategyTest, AddEqualsKeyMerge)
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

TEST_F(AutoMergeStrategyTest, EqualsAddKeyMerge)
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
