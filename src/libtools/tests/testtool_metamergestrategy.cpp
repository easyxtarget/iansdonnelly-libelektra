/**
 * \file
 *
 * \brief Tests for the MetaMergeStrategy
 *
 * \copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#include <gtest/gtest.h>
#include <merging/threewaymerge.hpp>
#include <merging/onesidestrategy.hpp>
#include <merging/metamergestrategy.hpp>
#include "mergetestutils.cpp"

using namespace std;
using namespace kdb;
using namespace kdb::tools::merging;

class MetaMergeStrategyTest : public MergeTest
{
protected:
	MergeResult result;
	MergeTask task;
	KeySet conflicts;

	MetaMergeStrategyTest() : task (MergeTask (BaseMergeKeys (base, baseParent),
				OurMergeKeys (ours, ourParent),
				TheirMergeKeys (theirs, theirParent), mergeParent))
	{
		result = MergeResult (conflicts, mergeKeys);
	}
};

TEST_F(MetaMergeStrategyTest, MergesMetaWithInnerStrategy)
{
	base.lookup ("user/parentb/config/key1").setMeta ("testmeta", "valueb");
	ours.lookup ("user/parento/config/key1").setMeta ("testmeta", "valueo");
	theirs.lookup ("user/parentt/config/key1").setMeta ("testmeta", "valuet");
	Key conflictKey = mk1;
	result.addConflict (conflictKey, META, META);
	conflictKey = result.getConflictSet ().at (0);

	ThreeWayMerge merger;
	merger.addConflictStrategy (new OneSideStrategy (OURS));
	MetaMergeStrategy metaStrategy (merger);
	metaStrategy.resolveConflict (task, conflictKey, result);

	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";
	KeySet merged = result.getMergedKeys ();
	cout << merged << endl;
	EXPECT_EQ(4, merged.size ());

	EXPECT_EQ("valueo", merged.lookup (mk1).getMeta<string> ("testmeta"));
}

// TODO: test conflict resolution

