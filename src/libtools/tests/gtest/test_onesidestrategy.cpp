/**
 * \file
 *
 * \brief Tests for the OneSideStrategy
 *
 * \copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#include <gtest/gtest.h>
#include <merging/onesidestrategy.hpp>
#include "mergetestutils.cpp"

using namespace kdb;
using namespace kdb::tools::merging;

class OneSideStrategyTest : public MergeTest
{
protected:
	MergeResult result;
	MergeTask task;
	KeySet conflicts;

	OneSideStrategyTest() : task (MergeTask (BaseMergeKeys (base, baseParent),
				OurMergeKeys (ours, ourParent),
				TheirMergeKeys (theirs, theirParent),
				mergeParent))
	{
		result = MergeResult(conflicts, mergeKeys);
	}
};

TEST_F(OneSideStrategyTest, BaseWinsCorrectly)
{
	base.lookup ("user/parentb/config/key1").setString("valueb");
	ours.lookup ("user/parento/config/key1").setString("valueo");
	theirs.lookup ("user/parentt/config/key1").setString("valuet");
	Key conflictKey = mergeKeys.lookup ("user/parentm/config/key1");
	result.addConflict (conflictKey, MODIFY, MODIFY);
	conflictKey = result.getConflictSet ().at (0);

	OneSideStrategy strategy(BASE);
	strategy.resolveConflict (task, conflictKey, result);

	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";
	KeySet merged = result.getMergedKeys ();
	cout << merged << endl;
	EXPECT_EQ(5, merged.size ());

	compareKeys (Key ("user/parentm/config/key1", KEY_VALUE, "valueb", KEY_END), merged.at(1));
}

TEST_F(OneSideStrategyTest, OursWinsCorrectly)
{
	base.lookup ("user/parentb/config/key1").setString("valueb");
	ours.lookup ("user/parento/config/key1").setString("valueo");
	theirs.lookup ("user/parentt/config/key1").setString("valuet");
	Key conflictKey = mergeKeys.lookup ("user/parentm/config/key1");
	result.addConflict (conflictKey, MODIFY, MODIFY);
	conflictKey = result.getConflictSet ().at (0);

	OneSideStrategy strategy(OURS);
	strategy.resolveConflict (task, conflictKey, result);

	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";
	KeySet merged = result.getMergedKeys ();
	cout << merged << endl;
	EXPECT_EQ(5, merged.size ());

	compareKeys (Key ("user/parentm/config/key1", KEY_VALUE, "valueo", KEY_END), merged.at(1));
}

TEST_F(OneSideStrategyTest, TheirsWinsCorrectly)
{
	base.lookup ("user/parentb/config/key1").setString("valueb");
	ours.lookup ("user/parento/config/key1").setString("valueo");
	theirs.lookup ("user/parentt/config/key1").setString("valuet");
	Key conflictKey = mergeKeys.lookup ("user/parentm/config/key1");
	result.addConflict (conflictKey, MODIFY, MODIFY);
	conflictKey = result.getConflictSet ().at (0);

	OneSideStrategy strategy(THEIRS);
	strategy.resolveConflict (task, conflictKey, result);

	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";
	KeySet merged = result.getMergedKeys ();
	cout << merged << endl;
	EXPECT_EQ(5, merged.size ());

	compareKeys (Key ("user/parentm/config/key1", KEY_VALUE, "valuet", KEY_END), merged.at(1));
}

