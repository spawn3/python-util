#!/usr/bin/env bash

set -x

create_new_cluster() {
    rm -rf default*.etcd

    nohup bin/etcd --name default1 --initial-cluster default1=http://localhost:2380,default2=http://localhost:22380,default3=http://localhost:32380 --initial-cluster-state new --initial-cluster-token etcd-cluster-01 --listen-client-urls http://localhost:2379 --advertise-client-urls http://localhost:2379 --listen-peer-urls http://localhost:2380 --initial-advertise-peer-urls http://localhost:2380 --data-dir default1.etcd &
    nohup bin/etcd --name default2 --initial-cluster default1=http://localhost:2380,default2=http://localhost:22380,default3=http://localhost:32380 --initial-cluster-state new --initial-cluster-token etcd-cluster-01 --listen-client-urls http://localhost:22379 --advertise-client-urls http://localhost:22379 --listen-peer-urls http://localhost:22380 --initial-advertise-peer-urls http://localhost:22380 --data-dir default2.etcd &
    nohup bin/etcd --name default3 --initial-cluster default1=http://localhost:2380,default2=http://localhost:22380,default3=http://localhost:32380 --initial-cluster-state new --initial-cluster-token etcd-cluster-01 --listen-client-urls http://localhost:32379 --advertise-client-urls http://localhost:32379 --listen-peer-urls http://localhost:32380 --initial-advertise-peer-urls http://localhost:32380 --data-dir default3.etcd &
}

member_add() {
    NUM_NODE=$1

    bin/etcdctl member add default${NUM_NODE} http://localhost:${NUM_NODE}2380

    CLUSTERS=""
    for((i=2; i <= ${NUM_NODE}; i++));do
        CLUSTER="default${i}=http://localhost:${i}2380"
        if [ "${CLUSTERS}"x = ""x ];then
            CLUSTERS="${CLUSTER}"
        else
            CLUSTERS="${CLUSTERS},${CLUSTER}"
        fi
        #echo "$i $CLUSTERS"
    done;

    CLUSTER="default1=http://localhost:2380"
    CLUSTERS="${CLUSTER},${CLUSTERS}"

    echo ${CLUSTERS}

    #exit 0

    export ETCD_NAME="default${NUM_NODE}"
    #export ETCD_INITIAL_CLUSTER="\"$CLUSTERS\""
    export ETCD_INITIAL_CLUSTER_STATE="existing"

    nohup bin/etcd --initial-cluster $CLUSTERS --listen-client-urls http://localhost:${NUM_NODE}2379 --advertise-client-urls http://localhost:${NUM_NODE}2379 --listen-peer-urls http://localhost:${NUM_NODE}2380 --initial-advertise-peer-urls http://localhost:${NUM_NODE}2380 --data-dir default${NUM_NODE}.etcd &
}

create_new_cluster

sleep 10
bin/etcdctl member list
member_add 4

sleep 10
bin/etcdctl member list
member_add 5

sleep 3
bin/etcdctl member list

#bin/etcdctl member add default5 http://localhost:52380

#export ETCD_NAME="default5"
#export ETCD_INITIAL_CLUSTER="default3=http://localhost:32380,default1=http://localhost:2380,default2=http://localhost:22380,default4=http://localhost:42380,default5=http://localhost:52380"
#export ETCD_INITIAL_CLUSTER_STATE="existing"

#nohup bin/etcd --listen-client-urls http://localhost:52379 --advertise-client-urls http://localhost:52379 --listen-peer-urls http://localhost:52380 --initial-advertise-peer-urls http://localhost:52380 --data-dir default5.etcd &

#bin/etcdctl member list
