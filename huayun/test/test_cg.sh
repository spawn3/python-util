#!/usr/bin/env bash

VOLS=/iscsi/pool1/vol11,/iscsi/pool1/vol12

test_create() {
    lich.snapshot --create $VOLS@snap01
    lich.snapshot --create $VOLS@snap02
    lich.snapshot --create $VOLS@snap03

    echo '=== snap01,snap02,snap03'
    lich.snapshot --list $VOLS

    lich.snapshot --rollback $VOLS@snap03

    lich.snapshot --remove $VOLS@snap01
    lich.snapshot --remove $VOLS@snap02
    lich.snapshot --remove $VOLS@snap03

    echo '=== empty'
    lich.snapshot --list $VOLS
}

test_protect() {
    lich.snapshot --create    $VOLS@snap01
    lich.snapshot --protect   $VOLS@snap01
    lich.snapshot --unprotect $VOLS@snap01
    lich.snapshot --remove    $VOLS@snap01
}

test_clone() {
    lich.snapshot --create $VOLS@snap01
    lich.snapshot --clone  $VOLS@snap01 /iscsi/pool1/vol11clone,/iscsi/pool1/vol12clone
    lich.snapshot --clone  $VOLS@snap01 /iscsi/pool2/vol11clone,/iscsi/pool2/vol12clone
    lich.snapshot --remove $VOLS@snap01
    lichfs --unlink /iscsi/pool1/vol11clone
    lichfs --unlink /iscsi/pool1/vol12clone
    lichfs --unlink /iscsi/pool2/vol11clone
    lichfs --unlink /iscsi/pool2/vol12clone
}

test_create
test_protect
test_clone
