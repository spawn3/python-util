#!/usr/bin/env bash

yum install -y net-snmp net-snmp-utils snmptt 
yum install -y mariadb-server mariadb
yum install -y zabbix22-server-mysql zabbix22 zabbix22-web-mysql zabbix22-agent

cp mibs/LICH-MIB.txt /usr/share/snmp/mibs/
