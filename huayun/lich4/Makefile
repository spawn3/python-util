centos_deps:
	dnf install -y libtool automake autoconf
	dnf install -y flex bison
	dnf install -y openssl-devel sqlite-devel libuuid-devel libaio-devel
	dnf install -y python-paramiko
	dnf install -y expect
	dnf install -y redhat-lsb
	dnf install -y gperftools

docker:
	dnf install -y docker
	systemctl start docker
