send:
	snmptrap -v 2c -c public localhost:162 "" LICH-MIB::trapEvent SNMPv2-MIB::sysLocation.0 s "just here"

cat:
	@cat /tmp/snmptrap.out

clean:
	@rm -f /tmp/snmptrap.out
