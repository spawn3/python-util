#!/bin/bash
read host
read ip
vars=

while read oid val
do
	if [ "$vars" = "" ]
	then
		vars="$oid = $val"
	else
		vars="$vars, $oid = $val"
	fi
done
echo trap: $host $ip $vars >/tmp/snmptrap.out
