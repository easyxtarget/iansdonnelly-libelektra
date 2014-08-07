# SECURITY #

Security is a very important point in librarys. In most use
cases there is nearly no point of danger in using elektra.
But some a very security related, especially when you use
a daemon or some kind of distributed configuration.


## Files and Environment Variables ##

system/ pathes are never effected by environment variables.
They always use the build-in KDB_DB_SYSTEM path.

user/ pathes, on the other hand, are resolved by:
 1.) metadata "owner", only to be modified by the program
 2.) the environment variable USER
     So in crontab scripts you should have
     export USER=<your name here>
     so that kdb works (if getlogin does not get the information from
     somewhere else - which is typically the case on linux systems)
 3.) Falls back to user "test".
     So if elektra tries to access e.g. /home/test/.kdb that typically
     means that USER is not set correctly, use
     export USER=<name here>
     in that script.
This owner is appended to KDB_DB_HOME.

All files below those pathes might be modified by elektra programs.
By making KDB_DB_SYSTEM world-writeable, the users might overwrite
the configuration of others.


## Compiler Options ##

Can be changed using standard CMake ways.
Some hints:

http://wiki.debian.org/Hardening



## Memory Leaks ##

We use valgrind (--tool=memcheck) to make sure that elektra
does not suffer memory leaks and incorrect memory handling.

