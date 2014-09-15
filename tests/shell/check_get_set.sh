@INCLUDE_COMMON@

echo
echo ELEKTRA BASIC COMMAND SCRIPTS TESTS
echo

check_version

VALUE=value

#override for specific testing
#PLUGINS=tcl

for PLUGIN in $PLUGINS
do
	if is_not_rw_storage
	then
		echo "$PLUGIN not a read-write storage"
		continue;
	fi

	case "$PLUGIN" in
	"tcl")
		MOUNT_PLUGIN="tcl ccode null"
		;;
	"ini")
		#TODO: broken?
		continue
		;;
	"yajl")
		MOUNT_PLUGIN="$PLUGIN"
		#TODO: add plugin to fix problem
		DO_NOT_TEST_ROOT_VALUE="yes"
		;;
	"simpleini")
		MOUNT_PLUGIN="simpleini ccode null"
		;;
	*)
		MOUNT_PLUGIN="$PLUGIN"
		;;
	esac

	unset -f cleanup
	FILE=test.$PLUGIN

	check_remaining_files $FILE

	$KDB mount $FILE $MOUNTPOINT $MOUNT_PLUGIN 1>/dev/null
	exit_if_fail "could not mount $FILE at $MOUNTPOINT using $MOUNT_PLUGIN"

	cleanup()
	{
		$KDB umount $MOUNTNAME >/dev/null
		succeed_if "could not umount $MOUNTNAME"
		rm -f $USER_FOLDER/$FILE
		rm -f $SYSTEM_FOLDER/$FILE

		USER_REMAINING="`find $USER_FOLDER -maxdepth 1 -name $FILE'*' -print -exec rm {} +`"
		test -z "$USER_REMAINING"
		succeed_if "found remaining files $USER_REMAINING in $USER_FOLDER"

		SYSTEM_REMAINING="`find $SYSTEM_FOLDER -maxdepth 1 -name $FILE'*' -print -exec rm {} +`"
		test -z "$SYSTEM_REMAINING"
		succeed_if "found remaining files $SYSTEM_REMAINING in $SYSTEM_FOLDER"
	}

	for ROOT in $USER_ROOT $SYSTEM_ROOT
	do
		echo "do preparation for $PLUGIN in $ROOT"
		$KDB set $ROOT "root" 1>/dev/null
		succeed_if "could not set root"

		[ "x`$KDB sget $ROOT/value defvalue 2> /dev/null`" = "xdefvalue" ]
		succeed_if "Did not get default value"

		if [ "x$DO_NOT_TEST_ROOT_VALUE" != "xyes" ]
		then
			#TODO: Fix by directoryvalue plugin
			[ "x`$KDB get $ROOT 2> /dev/null`" = "xroot" ]
			succeed_if "could not get root"

			[ "x`$KDB sget $ROOT default 2> /dev/null`" = "xroot" ]
			succeed_if "could not shell get root"
		fi

		$KDB set "$ROOT/value" "$VALUE" 1>/dev/null
		succeed_if "could not set value"

		[ "x`$KDB get $ROOT/value 2> /dev/null`" = "x$VALUE" ]
		succeed_if "cant get $ROOT/value"

		[ "x`$KDB sget $ROOT/value default 2> /dev/null`" = "x$VALUE" ]
		succeed_if "cant shell get $ROOT/value"

		echo "testing ls command"

		[ "x`$KDB ls $ROOT/value 2> /dev/null`" = "x$ROOT/value" ]
		succeed_if "cant ls $ROOT (may mean that $ROOT folder is not clean)"

		echo "testing rm command"

		$KDB rm $ROOT/value 1> /dev/null
		succeed_if "could not remove user/test/value"

		$KDB get $ROOT/value 1> /dev/null
		[ $? != "0" ]
		succeed_if "got removed key $ROOT/value"

		$KDB rm $ROOT 1>/dev/null
		succeed_if "could not remove user/test/value"

		[ "x`$KDB sget $ROOT/value value 2> /dev/null`" = "xvalue" ]
		succeed_if "Did not get default value after remove"

		$KDB get $ROOT/value 1>/dev/null
		[ $? != "0" ]
		succeed_if "got removed key $ROOT"

		check_set_rm $ROOT/value other_value

		echo "testing array"

		KEY=$ROOT/hello/a/key
		$KDB set "$KEY" "$VALUE" 1>/dev/null
		succeed_if "could not set key $KEY"

		[ "x`$KDB get $KEY`" = "x$VALUE" ]
		succeed_if "$KEY is not $VALUE"

		for i in `seq 0 9`
		do
			KEY="$ROOT/hello/a/array/#$i"
			VALUE="$i"

			$KDB set "$KEY" "$VALUE" 1>/dev/null
			succeed_if "could not set key $ROOT/hello/a/array/#0"

			[ "x`$KDB get $KEY`" = "x$VALUE" ]
			succeed_if "$KEY is not $VALUE"
		done

		$KDB rm -r "$ROOT"
		succeed_if "could not remove all keys"
	done

	cleanup
done

end_script basic commands
