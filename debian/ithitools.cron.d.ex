#
# Regular cron jobs for the ithitools package
#
0 4	* * *	root	[ -x /usr/bin/ithitools_maintenance ] && /usr/bin/ithitools_maintenance
