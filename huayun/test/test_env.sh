#!/usr/bin/env bash


build_env() {
    lichbd mkpool pool1 -p iscsi
    lichbd mkpool pool2 -p iscsi

    lichbd create pool1/vol11 -s 100M -p iscsi
    lichbd create pool1/vol12 -s 100M -p iscsi
    lichbd create pool1/vol13 -s 100M -p iscsi

    lichbd create pool2/vo211 -s 100M -p iscsi
    lichbd create pool2/vo212 -s 100M -p iscsi
    lichbd create pool2/vol13 -s 100M -p iscsi
}

build_env
