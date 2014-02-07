#from opt_support import *
#compiler-settings
directiveStartToken = @
cheetahVarStartToken = $
#end compiler-settings
// start of a generated file
#include "kdb.h"
#include <unistd.h>

// for strol
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>

int ksGetOpt(int argc, char **argv, KeySet *ks)
{
	int c;
	int retval = 0;
	opterr = 0;
	Key *found = 0;

	while ((c = getopt (argc, argv,
@for $key, $info in $parameters.items()
@if $info.get('opt'):
		"$info.get('opt'):"
@end if
@end for
		)) != -1)
	{
		switch (c)
		{
@for $key, $info in $parameters.items()
@if $info.get('opt'):
			case '$info.get("opt")':
@if $info.get('range')
				{
					$typeof(info) check;
					char *endptr;
					errno = 0;
					check = strtol(optarg, &endptr, 10);
					if ((errno == ERANGE
							&& (check == LONG_MAX || check == LONG_MIN))
							|| (errno != 0 && check == 0))
					{
						retval = 5;
						break;
					}
					if (endptr == optarg)
					{
						retval = 6;
						break;
					}
					if (check < $min(info))
					{
						retval = 3;
						break;
					}
					if (check > $max(info))
					{
						retval = 4;
						break;
					}
				}
@end if
				found = ksLookupByName(ks, "$userkey(key)", 0);
				if(!found)
				{
					ksAppendKey(ks, keyNew("$userkey(key)",
							KEY_VALUE, optarg,
							KEY_END));
				}
				else
				{
					keySetString(found, optarg);
				}
				break;
@end if
@end for
			case '?':
				retval = 1;
				break;
			default:
				retval = 2;
				break;
/*
			case '?':
				if (optopt == 'c')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr,
							"Unknown option character `\\x%x'.\n",
							optopt);
				return 1;
*/
		}
	}
	return retval;
}