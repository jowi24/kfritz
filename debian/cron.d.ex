#
# Regular cron jobs for the kfritzbox package
#
0 4	* * *	root	[ -x /usr/bin/kfritzbox_maintenance ] && /usr/bin/kfritzbox_maintenance
