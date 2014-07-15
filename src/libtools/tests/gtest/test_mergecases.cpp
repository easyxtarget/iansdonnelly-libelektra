/**
 * \file
 *
 * \brief Tests for the ThreeWayMerge
 *
 * \copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#include <iostream>
#include <gtest/gtest.h>
#include "mergetestutils.cpp"

using namespace kdb;
using namespace kdb::tools::merging;

class ThreeWayMergeTest : public MergeTest
{
protected:
	ThreeWayMerge merger;
};

TEST_F(ThreeWayMergeTest, EqualKeySetsMerge)
{

	MergeResult result = merger.mergeKeySet (base, ours, theirs, mergeParent);
	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";

	KeySet merged = result.getMergedKeys ();

	EXPECT_EQ(5, merged.size ());
	compareAllKeys (merged);

}

TEST_F(ThreeWayMergeTest, SameDeletedKeyMerge)
{

	ours.lookup ("user/parento/config/key1", KDB_O_POP);
	theirs.lookup ("user/parentt/config/key1", KDB_O_POP);
	theirs.lookup ("user/parentt/config/key1", KDB_O_POP);

	MergeResult result = merger.mergeKeySet (base, ours, theirs, mergeParent);
	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";

	KeySet merged = result.getMergedKeys ();

	EXPECT_EQ(4, merged.size ());
	compareAllExceptKey1 (merged);
}



TEST_F(ThreeWayMergeTest, DeleteModifyConflict)
{
	ours.lookup ("user/parento/config/key1", KDB_O_POP);
	theirs.lookup ("user/parentt/config/key1").setString ("modifiedvalue");

	MergeResult result = merger.mergeKeySet (base, ours, theirs, mergeParent);
	EXPECT_TRUE(result.hasConflicts()) << "No conflict detected although conflicts should exist";

	KeySet conflicts = result.getConflictSet ();
	ASSERT_EQ(1, conflicts.size())<< "Wrong number of conflicts";
	testConflictMeta (conflicts.at (0), DELETE, MODIFY);

	KeySet merged = result.getMergedKeys ();
	compareAllExceptKey1 (merged);
}

TEST_F(ThreeWayMergeTest, ModifyDeleteConflict)
{
	ours.lookup ("user/parento/config/key1").setString ("modifiedvalue");
	theirs.lookup ("user/parentt/config/key1", KDB_O_POP);

	MergeResult result = merger.mergeKeySet (base, ours, theirs, mergeParent);
	ASSERT_TRUE(result.hasConflicts())<< "No conflict detected although conflicts should exist";

	KeySet conflicts = result.getConflictSet ();
	EXPECT_EQ(1, conflicts.size()) << "Wrong number of conflicts";
	testConflictMeta (conflicts.at (0), MODIFY, DELETE);

	KeySet merged = result.getMergedKeys ();
	EXPECT_EQ(4, merged.size ());
	compareAllExceptKey1 (merged);
}


TEST_F(ThreeWayMergeTest, SameModifyConflict)
{
	ours.lookup ("user/parento/config/key1").setString ("modifiedvalueours");
	theirs.lookup ("user/parentt/config/key1").setString ("modifiedvaluetheirs");

	MergeResult result = merger.mergeKeySet (base, ours, theirs, mergeParent);
	ASSERT_TRUE (result.hasConflicts())<< "No conflict detected although conflicts should exist";

	KeySet conflicts = result.getConflictSet ();
	EXPECT_EQ(1, conflicts.size ());
	testConflictMeta (conflicts.at (0), MODIFY, MODIFY);

	KeySet merged = result.getMergedKeys ();

	EXPECT_EQ(4, merged.size ());
	compareAllExceptKey1 (merged);
}

TEST_F(ThreeWayMergeTest, SameAddedEqualValueMerges)
{
	ours.append (Key ("user/parento/config/key5", KEY_VALUE, "newvalue", KEY_END));
	theirs.append (Key ("user/parentt/config/key5", KEY_VALUE, "newvalue", KEY_END));

	MergeResult result = merger.mergeKeySet (base, ours, theirs, mergeParent);
	EXPECT_FALSE(result.hasConflicts()) << "Invalid conflict detected";

	KeySet merged = result.getMergedKeys ();

	EXPECT_EQ(6, merged.size ());
	compareAllKeys (merged);

	compareKeys (Key ("user/parentm/config/key5", KEY_VALUE, "newvalue", KEY_END), merged.at (5));
}

TEST_F(ThreeWayMergeTest, SameAddedDifferentValueConflict)
{
	ours.append (Key ("user/parento/config/key5", KEY_VALUE, "newvalueours", KEY_END));
	theirs.append (Key ("user/parentt/config/key5", KEY_VALUE, "newvaluetheirs", KEY_END));

	MergeResult result = merger.mergeKeySet (base, ours, theirs, mergeParent);

	ASSERT_TRUE (result.hasConflicts())<< "No conflict detected although conflicts should exist";

	KeySet conflicts = result.getConflictSet ();
	EXPECT_EQ(1, conflicts.size ());
	testConflictMeta (conflicts.at (0), ADD, ADD);

	KeySet merged = result.getMergedKeys ();

	EXPECT_EQ(5, merged.size ());

	compareAllKeys (merged);
}

TEST_F(ThreeWayMergeTest, SameMetaKeyModifyConflict)
{
	ours.lookup ("user/parento/config/key1").setMeta<std::string> ("testmeta", "ourvalue");
	theirs.lookup ("user/parentt/config/key1").setMeta<std::string> ("testmeta", "theirvalue");

	MergeResult result = merger.mergeKeySet (base, ours, theirs, mergeParent);

	ASSERT_TRUE (result.hasConflicts())<< "No conflict detected although conflicts should exist";
	KeySet conflicts = result.getConflictSet ();
	EXPECT_EQ(1, conflicts.size ());
	testConflictMeta (conflicts.at (0), META, META);
	KeySet merged = result.getMergedKeys ();
	compareAllExceptKey1 (merged);
}
