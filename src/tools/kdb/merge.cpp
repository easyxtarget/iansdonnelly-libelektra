#include <merge.hpp>

#include <kdb.hpp>
#include <modules.hpp>
#include <cmdline.hpp>
#include <keysetio.hpp>

#include <iostream>
#include <string>

#include <merging/threewaymerge.hpp>
#include <merging/metamergestrategy.hpp>
#include <mergehelper.hpp>

using namespace kdb;
using namespace kdb::tools::merging;
using namespace std;

MergeCommand::MergeCommand()
{
}

MergeCommand::~MergeCommand()
{

}

int MergeCommand::execute(Cmdline const& cl)
{

	if (cl.arguments.size () < 4)
	{
		throw invalid_argument ("wrong number of arguments, 4 needed");
	}

	Key oursRoot (cl.arguments[0], KEY_END);
	if (!oursRoot.isValid ())
	{
		throw invalid_argument (cl.arguments[0] + " is not a valid keyname");
	}

	Key theirsRoot (cl.arguments[1], KEY_END);
	if (!theirsRoot.isValid ())
	{
		throw invalid_argument (cl.arguments[1] + " is not a valid keyname");
	}

	Key baseRoot (cl.arguments[2], KEY_END);
	if (!baseRoot.isValid ())
	{
		throw invalid_argument (cl.arguments[2] + " is not a valid keyname");
	}

	Key resultRoot (cl.arguments[3], KEY_END);
	if (!baseRoot.isValid ())
	{
		throw invalid_argument (cl.arguments[3] + " is not a valid keyname");
	}

	KeySet ours;
	KeySet theirs;
	KeySet base;

	{
		KDB lkdb;
		lkdb.get (ours, oursRoot);
		ours = ours.cut (oursRoot);
		ours.lookup(oursRoot, KDB_O_POP);
		if (cl.verbose) std::cout << "we got ours: " << oursRoot << " with keys " << ours << std::endl;
	}
	{
		KDB lkdb;
		lkdb.get (theirs, theirsRoot);
		theirs = theirs.cut (theirsRoot);
		ours.lookup(oursRoot, KDB_O_POP);
		if (cl.verbose) std::cout << "we got theirs: " << theirsRoot << " with keys " << theirs << std::endl;
	}
	{
		KDB lkdb;
		lkdb.get (base, baseRoot);
		base = base.cut (baseRoot);
		ours.lookup(oursRoot, KDB_O_POP);
		if (cl.verbose) std::cout << "we got base: " << baseRoot << " with keys " << base << std::endl;
	}

	KeySet resultKeys;
	kdb.get (resultKeys, resultRoot);

	KeySet discard = resultKeys.cut (resultRoot);
	if (discard.size () != 0)
	{
		if (cl.force)
		{
			if (cl.verbose)
			{
				std::cout << "will remove " << discard.size () << " keys, because -f was given" << std::endl;
			}
		}
		else
		{
			std::cerr << discard.size ()
					<< " keys exist in merge resultroot, will quit. Use -f to override the keys there." << std::endl;
		}
	}

	MergeHelper helper;
	ThreeWayMerge merger;

	// TODO: for now we have to position this strategy manually
	// to avoid meta information loss
	MetaMergeStrategy metaStrategy(merger);
	merger.addConflictStrategy(&metaStrategy);

	helper.parseStrategies (cl, merger);

	MergeResult result = merger.mergeKeySet (
			MergeTask (BaseMergeKeys (base, baseRoot), OurMergeKeys (ours, oursRoot),
					TheirMergeKeys (theirs, theirsRoot), resultRoot));

	helper.reportResult (cl, result, cout, cerr);

	int ret = 0;
	if (!result.hasConflicts ())
	{
		resultKeys.append(result.getMergedKeys());
		kdb.set (resultKeys, resultRoot);
	}
	else
	{
		ret = -1;
	}

	return ret;
}

