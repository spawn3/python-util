#!/usr/bin/env bash

# PDCA
# centos 7

# install and init zabbix
#
# https://www.zabbix.com/documentation/2.2/manual/installation/install_from_packages
# https://www.zabbix.com/documentation/2.4/manual/config/items/itemtypes/snmptrap

# NOTE:
# zabbix host interface ip === 192.168.151.33


# LAMP
# php -i
yum install -y httpd
yum install -y mariadb-server mariadb

yum install -y zabbix22-server-mysql zabbix22 zabbix22-web-mysql

yum install -y zabbix22-agent

# SNMP
yum install -y net-snmp net-snmp-utils snmptt 

cp mibs/LICH-MIB.txt /usr/share/snmp/mibs/


# start services

# snmptrapd -d -On -Lo
# systemctl start snmptt
# systemctl start httpd
# systemctl start zabbix-server


# CHECK
# snmptrap -v 2c -c public 192.168.1.69:162 "" LICH-MIB::trapEvent SNMPv2-MIB::sysLocation.0 s "just here"
