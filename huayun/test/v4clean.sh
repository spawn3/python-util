#!/usr/bin/env bash

lich stop

for h in v1 v2 v3
do
    ssh $h "rm -rf /opt/fusionstack/etc/cluster.conf"
    ssh $h "pkill -9 lichd"
    ssh $h "rm -rf /opt/fusionstack/data"
    ssh $h "rm -rf /dev/shm/lich4/*"
done
