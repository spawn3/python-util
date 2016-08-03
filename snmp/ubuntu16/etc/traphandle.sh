#!/usr/bin/env bash

echo "$4

Host: $1
Severity: $2

Description:
$3
" | mail -s "$2 Error from: $1" noc@section6.net
