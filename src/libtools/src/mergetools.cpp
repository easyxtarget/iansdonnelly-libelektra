/**
 * \file
 *
 * \brief Implementation of mergetools
 *
 * \copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#include <mergetools.hpp>

#include <kdb.hpp>
#include <modules.hpp>
#include <keysetio.hpp>

#include <iostream>
#include <string>

using namespace std;

namespace kdb{

namespace tools{

Merge::Merge()
{}

/**
 * Returns the path of a key (fullname - basename).
 */
string Merge::getPath(const kdb::Key& key){
	unsigned found;
	string path;
	found = key.getName().find_last_of("/");
  	path = key.getName().substr(0,found+1);
	return path;
}

/**
 * Determines if two keys are equal based on their GetString() values.
 * Returns true if they are equal. False if they are not.
 */
bool Merge::KeyEqual(const kdb::Key& k1, const kdb::Key& k2){
	if(k1.getString() != k2.getString()){
		return false;
	}
	return true;
}

/**
 * Determines if two KeySets are equal.
 * KeySets are considered equal if they are the same size and both keysets have the same keys determined by their name and KeyEqual.
 */
bool Merge::KeySetEqual(const kdb::KeySet& ks1, const kdb::KeySet& ks2){
	if((ks1.size()) != (ks2.size()))
		return false;
	ks1.rewind();
	ks2.rewind();
	Key k1;
	Key k2;
	while(k1=ks1.next()){
		k2=ks2.next();
		if(k1.getBaseName() != k2.getBaseName()){
			//cerr << "Name mismatch!" << endl << "k1 name: " << k1.getBaseName() << " k2 name: " << k2.getBaseName() << endl;
			return false;
		}
		else if(!KeyEqual(k1, k2))
			return false;
	}
	return true;
}

/**
 * Returns a keyset that is the result of a merge on two keysets (ours and theirs) using a base keyset as a refernece (a three-way merge). 
 * If the merge function is unscuessful an empty KeySet will be returned. 
 * This function is inteded as a full version for the kdb merge command or for  the C++ API. 
 * It works by taking in three keysets, their parent keys and a parent key for where to store the merged KeySet.
**/
kdb::KeySet Merge::KeySetMerge(const kdb::KeySet& base, const kdb::Key& base_root, const kdb::KeySet& ours, const kdb::Key& our_root, const kdb::KeySet& theirs, const kdb::Key& their_root,  kdb::Key& merge_root){
	Key ourkey;
	Key theirkey;
	Key basekey;
	Key mergekey;
	KeySet merged;
	base.rewind();
	ours.rewind();
	theirs.rewind();

	
	string base_lookup;
	string our_lookup;
	string their_lookup;
	string base_path;
	string our_path;
	string their_path;
	string merge_path;

	//Get path of each keyset
  	base_path = base_root.getName() + "/";
  	our_path = our_root.getName() + "/";
  	their_path = their_root.getName() + "/";
	merge_path = merge_root.getName() + "/";


	//If our ks and their ks match
	if(KeySetEqual(ours, theirs)){
		//Set merged to ours
		while(ourkey = ours.next()){
			mergekey = ourkey.dup();
			mergekey.setName(merge_path + ourkey.getName().substr(our_path.length()));
			merged.append(mergekey);
		}
		return merged;
	}
	//If base ks and their ks match
	else if(KeySetEqual(base, theirs)){
		//Set merged to ours
		ours.rewind();
		while(ourkey = ours.next()){
			mergekey = ourkey.dup();
			mergekey.setName(merge_path + ourkey.getName().substr(our_path.length()));
			merged.append(mergekey);
		}
		return merged;
	}
	//If base ks and our ks match
	else if(KeySetEqual(base, ours)){
		//Set merged to theirs
		while(theirkey = theirs.next()){
			mergekey = theirkey.dup();
			mergekey.setName(merge_path + theirkey.getName().substr(their_path.length()));
			merged.append(mergekey);
		}
		return merged;
	}
	//If none of the keysets match
	else{
		//Iterate though ours and theirs and check each key. If keys don't match, refernece base.

		//Iterate over keysets.
		//Iterate over base first. Look for keys in all three KeySets and try to merge those keys.
		base.rewind();
		while(basekey=base.next()){
			base_lookup = base_path + basekey.getName().substr(base_path.length());
			our_lookup = our_path + basekey.getName().substr(base_path.length());
			their_lookup = their_path + basekey.getName().substr(base_path.length());
			//If a key exists in all three keysets, compare it and append it to merged unless their is a conflict.
			if(ours.lookup(our_lookup.c_str()) && theirs.lookup(their_lookup.c_str())){
				ourkey = ours.lookup(our_lookup.c_str());
				theirkey = theirs.lookup(their_lookup.c_str());
				if(KeyEqual(ourkey, theirkey)){
					mergekey = ourkey.dup();
					mergekey.setName(merge_path + ourkey.getName().substr(our_path.length()));
					merged.append(mergekey);
				}			
				else if(KeyEqual(basekey, theirkey)){
					mergekey = ourkey.dup();
					mergekey.setName(merge_path + ourkey.getName().substr(our_path.length()));
					merged.append(mergekey);
				}
				else if(KeyEqual(basekey, ourkey)){
					mergekey = theirkey.dup();
					mergekey.setName(merge_path + theirkey.getName().substr(their_path.length()));
					merged.append(mergekey);
				}
				else{
					//conflict!
					cerr << "Key content conflict! basekey = " << basekey << " ourkey = " << ourkey << " theirkey = " << theirkey << endl;
					return KeySet();
				}
			}
		}
	
		//Iterate over ours. If it has any unique keys, append them to merged.
		ours.rewind();
		while(ourkey=ours.next()){
			base_lookup = base_path + ourkey.getBaseName();
			our_lookup = our_path + ourkey.getBaseName();
			their_lookup = their_path + ourkey.getBaseName();
			//If ours has a key that theirs and base don't have.
			if(!base.lookup(base_lookup.c_str())){
				if(!theirs.lookup(their_lookup.c_str())){
					mergekey = ourkey.dup();
					mergekey.setName(merge_path + ourkey.getName().substr(our_path.length()));
					merged.append(mergekey);
				}
				//If base doesn't have it but theirs does.
				else{
					theirkey = theirs.lookup(their_lookup.c_str());
					if(!KeyEqual(ourkey, theirkey)){
						//Conflict!
						cerr << "Key content conflict, no base key, ourkey != theirkey. ourkey = " << ourkey << " theirkey = " << theirkey << endl;
						return KeySet();
					}
					else{
						mergekey = ourkey.dup();
						mergekey.setName(merge_path + ourkey.getName().substr(our_path.length()));
						merged.append(mergekey);
					}
				}
			}

		}

		//Iterate over theirs. If it has any unique keys, append them to merged.
		theirs.rewind();
		while(theirkey=theirs.next()){
			base_lookup = base_path + theirkey.getBaseName();
			our_lookup = our_path + theirkey.getBaseName();
			their_lookup = their_path + theirkey.getBaseName();
			//If theirs has a key that ours and base don't have.
			if(!base.lookup(base_lookup.c_str())){
				if(!ours.lookup(our_lookup.c_str())){
					mergekey = theirkey.dup();
					mergekey.setName(merge_path + theirkey.getName().substr(their_path.length()));
					merged.append(mergekey);
				}
				//If base doesn't have it but ours does.
				else{
					ourkey = ours.lookup(our_lookup.c_str());
					if(!KeyEqual(theirkey, ourkey)){
						//Conflict!
						cerr << "Key content conflict, no base key, ourkey != theirkey. ourkey = " << ourkey << " theirkey = " << theirkey << endl;
						return KeySet();
					}
				}
			}
		}
	}

	//Return merged
	return merged;
}

/**
 *
 * Returns a keyset that is the result of a merge on two keysets (ours and theirs) using a base keyset as a refernece (a three-way merge). 
 * If the merge function is unscuessful an empty KeySet will be returned. 
 * This function is inteded as a basic version for the C++ API. It takes in three keysets and a parent key for where to store the merged keys. 
 * It works by finidng the parent key for each keyset and then calling the more complex function above.
**/
kdb::KeySet Merge::KeySetMerge(const kdb::KeySet& base, const kdb::KeySet& ours, const kdb::KeySet& theirs, kdb::Key& merge_root){
	Key ourkey;
	Key theirkey;
	Key basekey;
	KeySet merged;
	base.rewind();
	ours.rewind();
	theirs.rewind();
	
	string base_lookup;
	string our_lookup;
	string their_lookup;
	string base_path;
	string our_path;
	string their_path;
	string merge_path;

	//Get path of each keyset
  	base_path = getPath(base.head());
  	our_path = getPath(ours.head());
  	their_path = getPath(theirs.head());

	basekey = base.tail().dup();
	ourkey = ours.tail().dup();
	theirkey = theirs.tail().dup();

	basekey.setName(getPath(basekey));
	ourkey.setName(getPath(ourkey));
	theirkey.setName(getPath(theirkey));

	merged = KeySetMerge(base, basekey, ours, ourkey, theirs, theirkey, merge_root);
	
	return merged;
}

Merge::~Merge()
{}

}

}

